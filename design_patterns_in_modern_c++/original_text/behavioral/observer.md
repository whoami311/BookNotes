# Observer

The observer pattern is a popular and necessary pattern, so it is surprising that, unlike other languages (e.g., C#), neither C++ nor the Standard Library come with a ready-to-use implementation. Nonetheless, a safe, properly implemented observer (if there can be such a thing) is a technically sophisticated construct, so in this chapter we’ll investigate it with all its gory details.

## Property Observers

People get old. It’s a fact of life. But when someone gets older by a year we might want to congratulate them on their birthday. But how? Given a definition such as:

```c++
struct Person
{
    int age;
    Person(int age) : age{age} {}
};
```

How do we know when a person’s age changes? We don’t. To see changes, we could try polling: reading a person’s age every 100 milliseconds and comparing the new value with the previous. This approach will work, but is tedious and does not scale. We need to be smarter about this.

We know that we want to be informed on every write to a person’s age field. Well, the only way to catch this is to make a setter, that is:

```c++
struct Person
{
    int get_age() const { return age; }
    void set_age(const int value) { age = value; }
private:
    int age;
};
```

The setter `set_age()` is where we can notify whoever cares that age has, in fact, changed. But how?

## Observer\<T\>

Well, one approach is to define some sort of base class that would need to be inherited by anyone interested in getting Person’s changes:

```c++
struct PersonListener
{
    virtual void person_changed(Person& p, const string& property_name, const any new_value) = 0;
};
```

However, this approach is quite stifling, because property changes can occur on types other than Person and we would not want to spawn additional classes for those too. Here’s something a little more generic:

```c++
template<typename T>
struct Observer
{
    virtual void field_changed(T& source, const string& field_name) = 0;
};
```

The two parameters in `field_changed()` are, hopefully, self-explanatory. The first is a reference to the object whose field actually changed, the second is the name of the field. Yes, the name is passed as a string, which does hurt the refactorability of our code (what if the field name changes?).

This implementation would allow us to observe changes to a Person class and, for example, write them to the command line:

```c++
struct ConsolePersonObserver : Observer<Person>
{
    void field_changed(Person& source, const string& field_name) override
    {
        cout << "Person's " << field_name << " has changed to " << source.get_age() << ".\n";
    }
};
```

The flexibility we introduced into the scenario would allow us, for example, to observe property changes on multiple classes. For instance, if we add class Creature to the mix, you can now observe on both:

```c++
struct ConsolePersonObserver : Observer<Person>, Observer<Creature>
{
    void field_changed(Person& source, ...) { ... };
    void field_changed(Creature& source, ...) { ... };
}
```

Another alternative is to use std::any and do away with a generic implementation. Try it!

## Observable\<T\>

So anyways, let’s get back to Person. Since this is about to become an observable class, it would have to take on new responsibilities, namely:

- Keeping a private list of all the observers interested in Person’s changes
- Letting the observers subscribe()/unsubscribe() to changes in Person
- Informing all observers when a change is actually made with notify()

All of this functionality can quite happily be moved to a separate base class so as to avoid replicating it for every potential observable:

```c++
template <typename T>
struct Observable
{
    void notify(T& source, const string& name) { ... }
    void subscribe(Observer<T>* f) { observers.push_back(f); }
    void unsubscribe(Observer<T>* f) { ... }
private:
    vector<Observer<T>*> observers;
};
```

We have implemented subscribe()—it just adds a new observer to the private list of observers. The list of observers isn’t available to anyone—not even to the derived class. We don’t want people arbitrarily manipulating this collection.

Next up, we need to implement notify(). The idea is simple: go through every observer and invoke its field_changed() one after another:

```c++
void notify(T& source, const string& name)
{
    for (auto obs : observers)
        obs->field_changed(source, name);
}
```

It’s not enough to inherit from Observable\<T\>, though: our class also needs to put do its part in calling notify() whenever a field is changed.

Consider the setter set_age(), for example. It now has three responsibilities:

- Check that the name has actually changed. If age is 20 and we are assigning 20 to it, there is no point performing any assignment or notification.
- Assign the field to the appropriate value.
- Call notify() with the right arguments.

Consequently, the new implementation of set_age() would look something like the following:

```c++
struct Person : Observable<Person>
{
    void set_age(const int age)
    {
        if (this->age == age) return ;
        this->age = age;
        notify(*this, "age");
    }
private:
    int age;
};
```

## Connecting Observers and Observables

We are now ready to start using the infrastructure we created in order to get notification on Person’s field changes (well, we could call them properties, really). Here’s a reminder of what our observer looks like:

```c++
struct ConsolePersonObserver : Observer<Person>
{
    void field_changed(Person& source, const string& field_name) override
    {
        cout << "Person's " << field_name << " has changed to " << source.get_age() << ".\n";
    }
};
```

And here is how we use it:

```c++
Person p{ 20 };
ConsolePersonObserver cpo;
p.suscribe(&cpo);
p.set_age(21); // Person's age has changed to 21.
p.set_age(22); // Person's age has changed to 22.
```

So long as you don’t concern yourself with issues around property dependencies and thread safety/reentrancy, you can stop here, take this implementation, and start using it. If you want to see discussions of more sophisticated approaches, read on.

## Dependency Problems

People aged 16 or older (could be different in your country) can vote. So suppose we want to be notified of changes to a person’s voting rights. First, let’s assume that our Person type has the following getter:

```c++
bool get_can_vote() const { return age >= 16; }
```

Note that `get_can_vote()` has no backing field and no setter (we could introduce such a field, but it would be self-evidently redundant), yet we also feel obliged to `notify()` on it. But how? Well, we could try to find what causes can_vote to change… that’s right, `set_age()` does! So if we want notifications on changes in voting status, these need to be done in `set_age()`. Get ready, you’re in for a surprise!

```c++
void set_age(int value) const
{
    if (age == value) return;

    auto old_can_vote = can_vote(); // store old value
    age = value;
    notify(*this, "age");

    if (old_can_vote != can_vote()) // check value has changed
        notify(*this, "can_vote");
}
```

There’s far too much in the preceding function. Not only do we check if age has changed, we also check that can_vote has changed and notify on it too! You can probably guess this approach doesn’t scale well, right? Imagine can_vote being dependent on two fields, say age and citizenship—it means both of their setters have to handle can_vote notifications. And what if age also affects ten other properties this way? This is an unworkable solution that would lead to brittle code that’s impossible to maintain, since relationships between variables need to be tracked manually.

Speaking plainly, in the preceding scenario, can_vote is a dependent property of age. The challenge of dependent properties is essentially the challenge of tools such as Excel: given lots of dependencies between different cells, how do you know which cells to recalculate when one of them changes.

Property dependencies can, of course, be formalized into some sort of `map<string, vector<string>>` that would keep a list of properties affected by a property (or, inversely, all the properties that affect a property). The sad thing is that this map would have to be defined by hand, and keeping it in sync with actual code is rather tricky.

## Unsubscription and Thread Safety

One thing that I’ve neglected to discuss is how an observer might `unsubscribe()` from an observable. Generally, you want to remove yourself from the list of observers, which in a single-threaded scenario is as simple as:

```c++
void unsubscribe(Observer<T>* observer)
{
    observer.earse(remove(observers.begin(), observers.end(), observer), observers.end());
};
```

While the use of the erase-remove idiom is technically correct, it is only correct in a single-threaded scenario. std::vector is not thread-safe, so calling, say, subscribe() and unsubscribe() at the same time could lead to unintended consequences, since both functions modify the vector.

This is easily cured: simply put a lock on all of observable’s operations. This can look as simple as:

```c++
template <typename T>
struct Observable
{
    void notify(T& source, const string& name)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
    void subscribe(Observer<T>* f)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
    void unsubscribe(Observer<T>* o)
    {
        scope_lock<mutex> lock{ mtx };
        ...
    }
private:
    vector<Observer<T>*> observers;
    mutex mtx;
};
```

Another, very viable, alternative is to use something like a concurrent_vector from TPL/PPL. Naturally you lose ordering guarantees (in other words, adding two objects one after another doesn’t guarantee they are notified in that order), but it certainly saves you from having to manage locks yourself.

## Reentrancy

The last implementation provides some thread safety through locking any of the three key methods when ever someone needs it. But now let’s imagine the following scenario: you have a TrafficAdministration component that keeps monitoring a person until they’re old enough to drive. When they’re 17, the component unsubscribes:

```c++
struct TrafficAdministration : Observer<Person>
{
    void TrafficAdministration::field_changed(Person& source, const string& filed_name) override
    {
        if (field_name == "age")
        {
            if (source.get_age() < 17)
                cout << "Whoa there, you are not old enough to drive!\n";
            else
            {
                // oh, ok, they are old enough, let's not monitor them anymore
                cout << "We no longer care!\n";
                source.unsubscribe(this);
            }
        }
    }
};
```

This is a problem because, when age turns to 17, the overall chain of calls will be:

```c++
notify() --> field_changed() --> unsubscribe()
```

This is a problem because in unsubscribe() we end up trying to take a lock that’s already taken. This is a reentrancy problem. There are different ways to handle this.

- One way is to simply prohibit such situations. After all, at least in this particular case, it’s very obvious that reentrancy is taking place here.
- Another way is to bail out on the idea of removing elements from the collection. Instead, we could go for something like:

```c++
void unsubscribe(Observer<T>* o)
{
    auto it != find(observers.begin(), observers.end(), o);
    if (it != observers.end())
        *it = nullptr;  // cannot do this for a set
}
```

And, subsequently, when you notify(), you just need an extra check:

```c++
void notify(T& source, const string& name)
{
    for (auto obs : observers)
    {
        if (obs)
            obs->field_changed(source, name);
    }
}
```

Of course, the preceding only solves possible contention between `notify()` and `subscribe()`. If you were to, for example, `subscribe()` and `unsubscribe()` at the same time, that’s still concurrent modification of collection—and it can still fail. So, at the very least, you might want to keep a lock there.

Yet another possibility is to just make a copy of the entire collection in notify(). You do need the lock still, you just don’t apply it to nofications. Here’s what I mean:

```c++
void notify(T& source, const string& name)
{
    vector<Observer<T>*> observers_copy;
    {
        lock_guard<mutex_t> lock{ mtx };
        observers_copy = observers;
    }
    for (auto obs : observers_copy)
        if (obs)
            obs->field_changed(source, name);
}
```

In the preceding implementation, we do take a lock but by the time we call field_changed, the lock has been released, since it’s only created in the artificial scope used to copy the vector. I wouldn’t worry about efficiency here, since a vector of pointers doesn’t take up that much memory.

Finally, it’s always possible to replace a mutex by a recursive_mutex. Generally speaking, recursive mutexes are hated by most developers (proof on SO), not just due to performance implications, but more due to the fact that in the majority of cases (just like Observer example), you can get away with using ordinary, nonrecursive variants if you design your code a bit better.

There are some interesting practical concerns that we haven’t really discussed here. They include the following:

- What happens if the same observer is added twice?
- If I allow duplicate observers, does `ubsubscribe()` remove every single instance?
- How is the behavior affected if we use a different container? For example, we decide to prevent duplicates by using an `std::set` or `boost::unordered_set`, what does this imply for ordinary operations?
- What if I want observers that are ranked by priority?

There and other practical concerns are all manageable once your foundations are solid. We won’t spend further time discussing them here.

## Observer via Boost.Signals2

There are many prepackaged implementation of the Observer pattern, and probably the most well known is the Boost.Signals2 library. Essentially, this library provides a type called signal that represents a signal in C++ terminology (called event elsewhere). This signal can be subscribed to by providing a function or lambda. It can also be unsubscribed to and, when you want to notify on this, it can be fired.

Using Boost.Signals2, we can define Observer\<T\> as follows:

```c++
template <typename T>
struct Observable
{
    signal<void(T&, const string&)> property_changed;
};
```

and its invocation looks as follows:

```c++
struct Person : Observable<Person>
{
    ...
    void set_age(const int age)
    {
        if (this->age == age) return;

        this->age = age;
        property_changed(*this, "age");
    }
};
```

The actual use of the API would directly use the signal unless, of course, you decided to add more API trappings to make it easier:

```c++
Person p{123};
auto conn = p.property_changed.connect([](Person&, const string& prop_name)
{
    cout << prop_name << " has been changed" << endl;
});
p.set_age(20);  // name has been changed

// later, optionally
conn.disconnect();
```

The result of a connect() call is a connection object that can also be used to unsubscribe when you no longer need notifications from the signal.

## Summary

Without a doubt, the code presented in this chapter is a clear example of overthinking and overengineering a problem way beyond what most people would want to achieve.

Let’s recap the main design decisions when implementing Observer:

- Decide what information you want your observable to communicate. For example, if you are handling field/property changes, you can include the name of the property. You can also specify old/new values, but passing the type could be problematic.
- Do you want your observers to been tire classes, or are you OK with just having a list of virtual functions?
- How do you want to handle observers unsubscribing?
    - If you don’t plan to support unsubscription—congratulations, you’ll save a lot of effort implementing the Observer, since there are no removal issues in reentrancy scenarios.
    - If you plan to support an explicit unsubscribe() function, you probably don’t want to erase-remove right in the function, but instead mark your elements for removal and remove them later.
    - If you don’t like the idea of dispatching on a (posibly null) raw pointer, consider using a weak_ptr instead.
- Is it likely that the functions of an Observer\<T\> will be invoked from several different threads? If they are, you need to protect your subscription list:
    - You can put a scoped_lock on all relevant functions; or
    - You can use a thread-safe collection such as the TBB/PPL concurrent_vector. You lose ordering guarantees.
- Are multiple subscriptions from the same source allowed? If they are, you cannot use an `std::set`.

There is, sadly, no ideal implementation of Observer that ticks all the boxes. Whichever implementation you go for, some compromises are expected.
