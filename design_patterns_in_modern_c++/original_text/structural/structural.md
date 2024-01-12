# Structural Patterns

As the name suggests, Structural patterns are all about setting up the structure of your application so as to improve SOLID conformance as well as general usability and refactorability of your code.

When it comes to determining the structure of an object, we can employ two fairly well-known methods:

- Inheritance: an object automatically acquires the nonprivate fields and functions of its base class or classes. To allow instantiation, the object must implement every pure virtual member from its parents; if it does not, it is abstract and cannot be created (but you can inherit from it).
- Composition: generally implies that the child cannot exist without the parent. Think of an object having members of `owner<T>` type: when the object gets destroyed, they get destroyed with it.
- Aggregation: an object can contain another object, but that object can also exist independently. Think of an object having members of type T* or `shared_ptr<T>`.

Nowadays, both composition and aggregation are treated in an identical fashion. If you have a Person class with a field of type Address, you have a choice as to whether Address is an external type or a nested type. In either case, provided it’s public, you can instantiate it as either Address or Person::Address.

I would argue that using the word composition when we really mean aggregation has become so commonplace that we may as well use them in interchangeable fashion. Here’s some proof: when we talk about IoC containers, we speak of a composition root. But wait, doesn’t the IoC container control the lifetime of each object individually? It does, and so we’re using the word “composition” when we really mean “aggregation” here.
