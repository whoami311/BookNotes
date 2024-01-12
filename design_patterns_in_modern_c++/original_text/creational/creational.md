# Creational Patterns

Even in the absence of creational patterns, the act of creation of an object in C++ is fraught with peril. Should you create on the stack or on the heap? Should that be a raw pointer, a unique or shared pointer, or something else entirely? Finally, is creating objects manually still kosher, or should we instead defer the creation of all key aspects of our infrastructure to specialized constructs such as Factories (more on them in just a moment!) or Inversion of Control containers?

Whichever option you choose, creation of objects can still be a chore, especially if the construction process is complicated or needs to abide by special rules. So that’s where creational patterns come from: they are common approaches related to the creation of objects.

Just in case you are rusty on your basic C++, or smart pointers in general, here’s a brief recap of the way objects are created in C++:

- *Stack allocation* creates an object that will be allocated on the stack. The object will be cleaned up automatically at the end of the scope (you can make an artificial scope anywhere with a pair of curly braces). The object will call the destructor at the very end of the scope provided you assign this object to a variable; if you don’t, the destructor will be called immediately. (This can ruin some implementations of the Memento design pattern, as we’ll discover later.)
- *Heap allocation* using a raw pointer puts the object on the heap (a.k.a. the free store). `Foo* foo = new Foo;` creates a new instance of Foo and leaves open the question of who is in charge of cleaning up the object. The GSL1 owner`<T>` tries to introduce some idea of “ownership” of a raw pointer but doesn’t involve any cleanup code—you still have to write it yourself.
- A *unique pointer* (unique_ptr) can take a heap-allocated pointer and manage it so that it’s cleaned up automatically when there is no longer a reference to it. A unique pointer really is unique: you cannot make copies of it, and you cannot pass it into another function without losing control of the original.
- A *shared pointer* (shared_ptr) takes a heap-allocated pointer and manages it, but allows the sharing of this pointer around in code. The owned pointer is only cleaned up when there are no components holding on to the pointer.
- A *weak pointer* (weak_ptr) is a smart but nonowning pointer, holding a weak reference to an object managed by a shared_ptr. You need to convert it to a shared_ptr
in order to be able to actually access the referenced object. One of its uses is to break circular references of shared_ptrs.

## Returning Objects From Functions

If you are returning anything bigger than a word-sized value, there are several ways of returning something from a function. The first, and most obvious, is:

```c++
Foo make_foo(int n)
{
    return Foo{n};
}
```

It may appear to you that, using the preceding, a full copy of Foo is being made, thereby wasting valuable resources. But it isn’t always so. Say you define Foo as:

```c++
struct Foo
{
    Foo(int n) {}
    Foo(const Foo&) { cout << "COPY CONSTRUCTOR!!!\n"; }
};
```

You will find that the copy constructor may be called anywhere from zero to two times: the exact number of calls depends on the compiler. Return Value Optimization (RVO) is a compiler feature that specifically prevents those extra copies being made (since they don’t really affect how the code behaves). In complex scenarios, however, you really cannot rely on RVO happening, but when it comes to choosing whether or not to optimize return values, I prefer to follow Knuth.

Another approach is, of course, to simply return a smart pointer such as a unique_ptr:

```c++
unique_ptr<Foo> make_foo(int n)
{
    return make_unique<Foo>(n);
}
```

This is very safe, but also opinionated: you’ve chosen the smart pointer for the user. What if they don’t like smart pointers? What if they would prefer a shared_ptr instead?

The third and final option is to use a raw pointer, perhaps in tandem with GSL’s owner`<T>`. This way, you are not enforcing the clean-up of the allocated object, but you are sending a very clear message that it is the caller’s responsibility:

```c++
owner<Foo*> make_foo(int n)
{
    return new Foo(n);
}
```

You can consider this approach as giving the user a hint: I’m returning a pointer and it’s up to you to take care of the pointer from now on. Of course, now the caller of make_foo() needs to handle the pointer: either by correctly calling delete or by wrapping it in a unique_ptr or shared_ptr. Keep in mind that owner`<T>` says nothing about copying.

All of these options are equally valid, and it’s difficult to say which option is better.
