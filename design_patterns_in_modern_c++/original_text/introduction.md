# Design Patterns in Modern C++

## Chapter 1

### Introduction

The topic of Design Patterns sounds dry, academically constipated and, in all honesty, done to death in almost every programming language imaginable—including programming languages such as JavaScript that aren’t even properly OOP! So why another book on it?

I guess the main reason this book exists is that C++ is great again. After a long period of stagnation, it’s now evolving, growing, and despite the fact that it has to contend with backwards C compatibility, good things are happening, albeit not at the pace we’d all like. (I’m looking at modules, among other things.)

Now, on to Design Patterns—we shouldn’t forget that the original Design Patterns book1 was published with examples in C++ and Smalltalk. Since then, plenty of programming languages have incorporated design patterns directly into the language: for example, C# directly incorporated the Observer pattern with its built-in support for events (and the corresponding event keyword). C++ has not done the same, at least not on the syntax level. That said, the introduction of types such as std::function sure made things a lot simpler for many programming scenarios.

Design Patterns are also a fun investigation of how a problem can be solved in many different ways, with varying degrees of technical sophistication and different sorts of trade-offs. Some patterns are more or less essential and unavoidable, whereas other patterns are more of a scientific curiosity (but nevertheless will be discussed in this book, since I’m a completionist).

Readers should be aware that comprehensive solutions to certain problems (e.g., the Observer pattern) typically result in overengineering, that is, the creation of structures that are far more complicated than is necessary for most typical scenarios. While overengineering is a lot of fun (hey, you get to really solve the problem and impress your coworkers), it’s often not feasible.

### Preliminaries

#### Who This Book Is For

This book is designed to be a modern-day update to the classic GoF book, targeting specifically the C++ programming language. I mean, how many of you are writing Smalltalk out there? Not many; that would be my guess.

The goal of this book is to investigate how we can apply Modern C++ (the latest versions of C++ currently available) to the implementations of classic design patterns. At the same time, it’s also an attempt to flesh out any new patterns and approaches that could be useful to C++ developers.

Finally, in some places, this book is quite simply a technology demo for Modern C++, showcasing how some of its latest features (e.g., coroutines) make difficult problems a lot easier to solve.

#### On Code Examples

The examples in this book are all suitable for putting into production, but a few simplifications have been made in order to aid readability:

- Quite often, you’ll find me using struct instead of class in order to avoid writing the public keyword in too many places.
- I will avoid the std:: prefix, as it can hurt readability, especially in places where code density is high. If I’m using string, you can bet I’m referring to std::string.
- I will avoid adding virtual destructors, whereas in real life, it might make sense to add them.
- In very few cases I will create and pass parameters by value to avoid the proliferation of shared_ptr/make_shared/etc. Smart pointers add another level of complexity, and their integration into the design patterns presented in this book is left as an exercise for the reader.
- I will sometimes omit code elements that would otherwise be necessary for feature-completing a type (e.g., move constructors) as those take up too much space.
- There will be plenty of cases where I will omit const whereas, under normal circumstances, it would actually make sense. Const-correctness quite often causes a split and a doubling of the API surface, something that doesn’t work well in book format.

You should be aware that most of the examples leverage Modern C++ (C++11, 14, 17 and beyond) and generally use the latest C++ language features that are available to developers. For example, you won’t find many function signatures ending in -> decltype(...) when C++14 lets us automatically infer the return type. None of the examples target a particular compiler, but if something doesn’t work with your chosen compiler, you’ll need to find workarounds.

At certain points in time, I will be referencing other programming languages such as C# or Kotlin. It’s sometimes interesting to note how designers of other languages have implemented a particular feature. C++ is no stranger to borrowing generally available ideas from other languages: for example, the introduction of auto and type inference on variable declarations and return types is present in many other languages.

#### Prince

Digital piracy is an inescapeable fact of life. A brand new generation is growing up right now that has never purchased a movie or a book—even this book. There’s not much that can be done about this. The only thing I can say is that if you pirated this book, you might not be reading the latest version.

The joy of online digital publishing is I get to update the book as new versions of C++ come out and I do more research. So if you paid for this book, you’ll get free updates in the future as new versions of the C++ language and the Standard Library are released. If not… oh, well.

### Important Concepts

Before we begin, I want to briefly mention some key concepts of the C++ world that are going to be referenced in this book.

#### Curiously Recurring Template Pattern

Hey, this is a pattern, apparently! I don’t know if it qualifies to be listed as a separate design pattern, but it’s certainly a pattern of sorts in the C++ world. Essentially, the idea is simple: an inheritor passes itself as a template argument to its base class:

```c++
struct Foo : SomeBase<Foo>
{
    ...
}
```

Now, you might be wondering why one would ever do that? Well, one reason is to be able to access a typed this pointer inside a base class implementation.

For example, suppose every single inheritor of SomeBase implements a begin()/end() pair required for iteration. How can you iterate the object inside a member of SomeBase? Intuition suggests that you cannot, because SomeBase itself does not provide a begin()/end() interface. But if you use CRTP, you can actually cast this to a derived class type:

```c++
template <typename Derived>
struct SomeBase
{
    void foo()
    {
        for (auto& item : *static_cast<Derived*>(this))
        {
            ...
        }
    }
}
```

For a concrete example of this approach, check out Chapter 9.

#### Mixin Inheritance

In C++, a class can be defined to inherit from its own template argument, for example:

```c++
template <typename T> struct Mixin : T
{
    ...
}
```

This approach is called mixin inheritance and allows hierarchical composition of types. For example, you can allow `Foo<Bar<Baz>> x;` to declare a variable of a type that implements the traits of all three classes, without having to actually construct a brand new FooBarBaz type.

For a concrete example of this approach, check out Chapter 9.

#### Properties

A property is nothing more than a (typically private) field and a combination of a getter and a setter. In standard C++, a property looks as follows:

```c++
class Person
{
    int age;
public:
    int get_age() const { return age; }
    void set_age(int value) { age = value; }
};
```

Plenty of languages (e.g., C#, Kotlin) internalize the notion of a property by baking it directly into the programming language. While C++ has not done this (and is unlikely to do so anytime in the future), there is a nonstandard declaration specifier called property that you can use in most compilers (MSVC, Clang, Intel):

```c++
class Person
{
    int age_;
public:
    int get_age() const { return age_; }
    void set_age(int value) { age_ = value; }
    __declspec(property(get=get_age, put=set_age)) int age;
};
```

This can be used as follows:

```c++
Person person;
p.age = 20; // calls p.set_age(20)
```

### The SOLID Design Principles

SOLID is an acronym which stands for the following design principles (and their abbreviations):

- Single Responsibility Principle (SRP)
- Open-Closed Principle (OCP)
- Liskov Substitution Principle (LSP)
- Interface Segregation Principle (ISP)
- Dependency Inversion Principle (DIP)

These principles were introduced by Robert C. Martin in the early 2000s—in fact, they are just a selection of five principles out of dozens that are expressed in Robert’s books and his blog. These five particular topics permeate the discussion of patterns and software design in general, so before we dive into design patterns (I know you’re all eager), we’re going to do a brief recap of what the SOLID principles are all about.

#### Single Responsibility Principle

Suppose you decide to keep a journal of your most intimate thoughts. The journal has a title and a number of entries. You could model it as follows:

```c++
struct Journal
{
    string title;
    vector<string> entries;

    explicit Journal(const string& title) : title(title) {}
};
```

Now, you could add functionality for adding an entry to the journal, prefixed by the entry’s ordinal number in the journal. This is easy:

```c++
void Journal::add(const string& entry)
{
    static int count = 1;
    entries.push_back(boost::lexical_cast<string>(count++) + ": " + entry);
}
```

And the journal is now usable as:

```c++
Journal j("Dear Diary");
j.add("I cried today");
j.add("I ate a bug");
```

It makes sense to have this function as part of the Journal class because adding a journal entry is something the journal actually needs to do. It is the journal’s responsibility to keep entries, so anything related to that is fair game.

Now suppose you decide to make the journal persist by saving it in a file. You add this code to the Journal class:

```c++
void Journal::save(const string& filename)
{
    ofstream ofs(filename);
    for (auto& s : entries)
        ofs << s << endl;
}
```

This approach is problematic. The journal’s responsibility is to keep journal entries, not to write them to disk. If you add the disk-writing functionality to Journal and similar classes, any change in the approach to persistence (say, you decide to write to the cloud instead of disk) would require lots of tiny changes in each of the affected classes.

I want to pause here and make a point: an architecture that leads you to having to do lots of tiny changes in lost of classes, whether related (as in a hierarchy) or not, is typically a code smell—an indication that something’s not quite right. Now, it really depends on the situation: if you’re renaming a symbol that’s being used in a hundred places, I’d argue that’s generally OK because ReSharper, CLion, or whatever IDE you use will actually let you perform a refactoring and have the change propagate everywhere. But when you need to completely rework an interface… well, that can be a very painful process!

I therefore state that persistence is a separate concern, one that is better expressed in a separate class, for example:

```c++
struct PersistenceManager
{
    static void save(const Journal& j, const string& filename)
    {
        ofstream ofs(filename);
        for (auto& s : entries)
            ofs << s << endl;
    }
};
```

This is precisely what is meant by Single Responsibility: each class has only one responsibility, and therefore has only one reason to change. Journal would need to change only if there’s something more that needs to be done with respect to storage of entries—for example, you might want each entry prefixed by a timestamp, so you would change the add() function to do exactly that. On the other hand, if you wanted to change the persistence mechanic, this would be changed in PersistenceManager.

An extreme example of an antipattern that violates the SRP is called a God Object. A God Object is a huge class that tries to handle as many concerns as possible, becoming a monolithic monstrosity that is very difficult to work with.

Luckily for us, God Objects are easy to recognize and thanks to source control systems (just count the number of member functions), the responsible developer can be quickly identified and adequately punished.

#### Open-Closed Principle

Suppose we have an (entirely hypothetical) range of products in a database. Each product has a color and size and is defined as:

```c++
enum class Color { Red, Green, Blue };
enum class Size { Small, Medium, Large };

struct Product
{
    string name;
    Color color;
    Size size;
};
```

Now, we want to provide certain filtering capabilities for a given set of products. We make a filter similar to the following:

```c++
struct ProductFilter
{
    typedef vector<Product*> Items;
};
```

Now, to support filtering products by color, we define a member function to do exactly that:

```c++
ProductFilter::Items ProductFilter::by_color(Items items, Color color)
{
    Items result;
    for (auto& i : items)
        if (i->color == color)
            result.push_back(i);
    return result;
}
```

Our current approach of filtering items by color is all well and good. Our code goes into production but, unfortunately, some time later the boss comes in and asks us to implement filtering by size, too. So we jump back into ProductFilter.cpp, add the following code and recompile:

```c++
ProductFilter::Items ProductFilter::by_color(Items items, Color color)
{
    Items result;
    for (auto& i : items)
        if (i->color == color)
            result.push_back(i);
    return result;
}
```

This feels like outright duplication, doesn’t it? Why don’t we just write a general method that takes a predicate (some function)? Well, one reason could be that different forms of filtering can be done in different ways: for example, some record types might be indexed and need to be searched in a specific way; some data types are amenable to search on a GPU, while others are not.

Our code goes into production but, once again, the boss comes back and tells us that now there’s a need to search by both color and size. So what are we to do but add another function?

```c++
ProductFilter::Items ProductFilter::by_color_and_size(Items items, Size size, Color color)
{
    Items result;
    for(auto& i : items)
        if (i->size == size && i->color == color)
            result.push_back(i);
    return result;
}
```

What we want, from the preceding scenario, is to enfoce the Open-Closed Principle that states that a type is open for extension but closed for modification. In other words, we want filtering that is extensible (perhaps in a different compilation unit) without having to modify it (and recompiling something that already works and may have been shipped to clients).

How can we achieve it? Well, first of all, we conceptually separate (SRP!) our filtering process into two parts: a filter (a process which takes all items and only returns some) and a specification (the definition of a predicate to apply to a data element).

We can make a very simple definition of a specification interface:

```c++
template <typename T> struct Specification
{
    virtual bool is_satisfied(T* item) = 0;
};
```

In the preceding example, type T is whatever we choose it to be: it can certainly be a Product, but it can also be something else. This makes the entire approach reusable.

Next up, we need a way of filtering based on `Specification<T>`: this is done by defining, you guessed it, a `Filter<T>`:

```c++
template <typename T> struct Filter
{
    virtual vector<T*> filter(
        vector<T*> items,
        Specification<T>& spec) = 0;
};
```

Again, all we are doing is specifying the signature for a function called filter which takes all the items and a specification, and returns all items that conform to the specification. There is an assumption that the items are stored as a vector<T*>, but in reality you could pass filter() either a pair of iterators or some custom-made interface designed specifically for going through a collection. Regrettably, the C++ language has failed to standardize the notion of an enumeration or collection, something that exists in other programming languages (e.g., .NET’s IEnumerable).

Based on the preceding, the implementation of an improved filter is really simple:

```c++
struct BetterFilter : Filter<Product>
{
    vector<Product*> filter(
        vector<Product*> items,
        Specification<Product>& spec) override
    {
        vector<Product*> result;
        for (auto& p : items)
            if (spec.is_satisfied(p))
                result.push_back(p);
        return result;
    }
};
```

Again, you can think of a `Specification<T>` that’s being passed in as a strongly typed equivalent of an std::function that is constrained only to a certain number of possible filter specifications.

Now, here’s the easy part. To make a color filter, you make a ColorSpecification:

```c++
struct ColorSpecification : Specification<Product>
{
    Color color;

    explicit ColorSpecification(const Color color) : color(color) {}

    bool is_satisfied(Product* item) override {
        return item->color == color;
    }
};
```

Armed with this specification, and given a list of products, we can now filter them as follows:

```c++
Product apple{ "Apple", Color::Green, Size::Small };
Product tree{ "Tree", Color::Green, Size::Large };
Product house{ "House", Color::Blue, Size::Large };

vector<Product*> all{ &apple, &tree, &house };

BetterFilter bf;
ColorSpecification green(Color::Green);

auto green_things = bf.filter(all, green);
for (auto& x : green_things)
    cout << x->name << " is green" << endl;
```

The preceding gets us “Apple” and “Tree” because they are both green. Now, the only thing we haven’t implemented so far is searching for size and color (or, indeed, explained how you would search for size or color, or mix different criteria). The answer is that you simply make a composite specification. For example, for the logical AND, you can make it as follows:

```c++
template <typename T> struct AndSpecification : Specification<T>
{
    Specification<T>& first;
    Specification<T>& second;

    AndSpecification(Specification<T>& first, Specification<T>& second) : first{first}, second{second} {}

    bool is_satisfied(T* item) override
    {
        return first.is_satisfied(item) && second.is_satisfied(item);
    }
};
```

Now, you are free to create composite conditions on the basis of simpler Specifications. Reusing the green specification we made earier, finding something green and big is now as simple as:

```c++
SizeSpecification large(Size::Large);
ColorSpecification green(Color::Green);
AndSpecification green_and_large{ large, green };

auto big_green_things = bf.filter(all, green_and_big);
for (auto& x : big_green_things)
    cout << x->name << " is large and green" << endl;

// Tree is large and green
```

This was a lot of code! But keep in mind that, thanks to the power of C++, you can simply introduce an operator && for two `Specification<T>` objects, thereby making the process of filtering by two (or more!) criteria extremely simple:

```c++
template <typename T> struct Specification
{
    virtual bool is_saticfied(T* item) = 0;

    AndSpecification<T> operator &&(Specification&& other)
    {
        return AndSpecification<T>(*this, other);
    }
};
```

If you now avoid making extra variables for size/color specifications, the composite specification can be reduced to a single line:

```c++
auto green_and_big = ColorSpecification(Color::Green) && SizeSpecification(Size::Large);
```

So let’s recap what OCP principle is and how the preceding example enforces it. Basically, OCP states that you shouldn’t need to go back to code you’ve already written and tested and change it. And that’s exactly what’s happening here! We made `Specification<T>` and `Filter<T>` and, from then on, all we have to do is implement either of the interfaces (without modifying the interfaces themselves) to implement new filtering mechanics. This is what is meant by “open for extension, closed for modification.”

#### Liskov Substitution Principle

The Liskov Substitution Principle, named after Barbara Liskov, states that if an interface takes an object of type Parent, it should equally take an object of type Child without anything breaking. Let’s take a look at a situation where LSP is broken.

Here’s a rectangle; it has width and height and a bunch of getters and setters calculating the area:

```c++
class Rectangle
{
protected:
    int width, height;
public:
    Rectangle(const int width, const int height) : width{width}, height{height} {}

    int get_width() const { return width; }
    virtual void set_width(const int width) { this->width = width; }
    int get_height() const { return height; }
    virtual void set_height(const int height) { this->height = height; }

    int area() const { return width * height; }
};
```

Now let’s suppose we make a special kind of Rectangle called a Square. This object overrides the setters to set both width and height:

```c++
class Square : public Rectangle
{
public:
    Square(int size) : Rectangle(size, size) {}
    void set_width(const int width) override {
        this->width = height = width;
    }
    void set_height(const int height) override {
        this->height = width = height;
    }
};
```

This approach is evil. You cannot see it yet, because it looks very innocent indeed: the setters simply set both dimensions, what could possibly go wrong? Well, if we take the preceding, we can easily construct a function taking a Rectangle that would blow up when taking a square:

```c++
void process(Rectangle& r)
{
    int w = r.get_width();
    r.set_height(10);

    cout << "expected area = " << (w * 10) << ", got " << r.area() << endl;
}
```

The preceding function takes the formula Area = Width × Height as an invariant. It gets the width, sets the height, and rightly expects the product to be equal to the calculated area. But calling the preceding function with a Square yields a mismatch:

```c++
Square s{5};
process(s); // expected area 50, got 25
```

The takeaway from this example (which I admit is a little contrived) is that process() breaks the LSP by being thoroughly unable to take a derived type Square instead of the base type Rectangle. If you feed it a Rectangle, everything is fine, so it might take some time before the problem shows up in your tests (or in production—hopefully not!).

What’s the solution? Well, there are many. Personally, I’d argue that the type Square shouldn’t even exist: instead, we can make a Factory (see Chapter 3) that creates both rectangles and squares:

```c++
struct RectangleFactory
{
    static Rectangle create_rectangle(int w, int h);
    static Rectangle create_square(int size);
};
```

You might also want a way of detecting that a Rectangle is, in fact, a square:

```c++
bool Rectangle::is_square() const
{
    return width = height;
}
```

The nuclear option, in this case, would be to throw an exception in Square’s `set_width()/set_height()`, stating that these operations are unsupported and you should be using `set_size()` instead. This, however, violates the principle of least surpise, since you would expect a call to `set_width()` to make a meaningful change… am I right?

#### Interface Segregation Principle

OK, here is another contrived example that is nonetheless suitable for illustrating the problem. Suppose you decide to define a multifunction printer: a device that can print, scan, and also fax documents. So you define it like so:

```c++
struct MyFavouritePrinter /* : IMachine */
{
    void print(vector<Document*> docs) override;
    void fax(vector<Document*> docs) override;
    void scan(vector<Document*> docs) override;
};
```

This is fine. Now, suppose you decide to define an interface that needs to be implemented by everyone who also plans to make a multifunction printer. So you could use the Extract Interface function in your favourite IDE and you’d get something like the following:

```c++
struct IMachine {
    virtual void print(vector<Document*> docs) = 0;
    virtual void fax(vector<Document*> docs) = 0;
    virtual void scan(vector<Document*> docs) = 0;
}
```

This is a problem. The reason it is a problem is that some implementor of this interface might not need scanning or faxing, just printing. And yet, you are forcing them to implement those extra features: sure, they can all be no-op, but why bother with this?

So what the ISP suggests is that you split up interfaces so that implementors can pick and choose depending on their needs. Since printing and scanning are different operations (for example, a Scanner cannot print), we define separate interfaces for these:

```c++
struct IPrinter
{
    virtual void print(vector<Document*> docs) = 0;
};

struct IScanner
{
    virtual void scan(vector<Document*> docs) = 0;
};
```

Then, a printer or a scanner can just implement the required functionality:

```c++
struct Printer : IPrinter
{
    void print(vector<Document*> docs) override;
};

struct Scanner : IScanner
{
    void scan(vector<Document*> docs) override;
};
```

Now, if we really want an IMachine interface, we can define it as a combination of the aforementioned interfaces:

```c++
struct IMachine: IPrinter, IScanner /* IFax and so on */
{
};
```

And when you come to implement this interface in your concrete multifunction device, this is the interface to use. For example, you could use simple delegation to ensure that Machine reuses the functionality provided by a particular IPrinter and IScanner:

```c++
struct Machine : IMachine
{
    IPrinter& printer;
    IScanner& scanner;

    Machine(IPrinter& printer, IScanner& scanner)
        : printer{printer},
          scanner{scanner}
    {
    }

    void print(vector<Document*> docs) override {
        printer.print(docs);
    }

    void scan(vector<Document*> docs) override {
        scanner.scan(docs);
    }
};
```

So, just to recap, the idea here is to segregate parts of a complicated interface into separate interfaces so as to avoid forcing implementors to implement functionality that they do not really need. Anytime you write a plug-in for some complicated application and you’re given an interface with 20 confusing functions to implement with various no-ops and return `nullptr`, more likely than not the API authors have violated the ISP.

#### Dependency Inversion Principle

The original definition of the DIP states the following:

A. High-level modules should not depend on low-level modules. Both should depend on abstractions.

What this statement basically means is that, if you’re interested in logging, your reporting component should not depend on a concrete ConsoleLogger, but can depend on an ILogger interface. In this case, we are considering the reporting component to be high level (closer to the business domain), whereas logging, being a fundamental concern (kind of like file I/O or threading, but not quite) is considered a low-level module.

B. Abstractions should not depend on details. Details should depend on abstractions.

This is, once again, restating that dependencies on interfaces or base classes are better than dependencies on concrete types. Hopefully the truth of this statement is obvious, because such an approach supports better configurability and testability—provided you’re using a good framework to handle these dependencies for you.

So now the main question is: how do you actually implement all of the preceding? It surely is a lot more work, because now you need to explicitly state that, for example, Reporting depends on an ILogger. The way you would express it is perhaps as follows:

```c++
class Reporting
{
    ILogger& logger;
public:
    Reporting(const ILogger& logger) : logger{logger} {}
    void prepare_report()
    {
        logger.log_info("Preparing the report");
        ...
    }
};
```

Now the problem is that, to initialize the preceding class, you need to explicitly call Reporting{ConsoleLogger{}} or something similar. And what if Reporting is dependent upon five different interfaces? What if ConsoleLogger has dependencies of its own? You can manage this by writing a lot of code, but there is a better way.

The modern, trendy, fashionable way of doing the preceding is to use Dependency Injection: this essentially means that you use a library such as Boost.DI to automatically satisfy the dependency requirements for a particular component.

Let’s consider an example of a car that has an engine but also needs to write to a log. As it stands, we can say that a car depends on both of these things. To start with, we may define an engine as:

```c++
struct Engine
{
    float volume = 5;
    int horse_power = 400;

    friend ostream& operator<< (ostream& os, Engine& obj)
    {
        return os
            << "volume: " << obj.volume
            << "horse_power: " << obj.horse_power;
    }   // thanks, ReSharper!
};
```

Now, it’s up to us to decide whether or not we want to extract an IEngine interface and feed it to the car. Maybe we do, maybe we don’t, and this is typically a design decision. If you envision having a hierarchy of engines, or you foresee needing a NullEngine (see Chapter 19) for testing purposes, then yes, you do need to abstract away the interfaces.

At any rate, we also want logging, and since this can be done in many ways (console, email, SMS, pigeon mail,…), we probably want to have an ILogger interface:

```c++
struct ILogger
{
    virtual ~ILogger() {}
    virtual void Log(const string& s) = 0;
};
```

as well as some sort of concrete implementation:

```c++
struct ConsoleLogger : ILogger
{
    ConsoleLogger() {}

    void Log(cosnt string& s) override
    {
        cout << "LOG: " << s.c_str() << endl;
    }
};
```

Now, the car we’re about to define depends on both the engine and the logging component. We need both, but it’s really up to us how to store them: we can use a pointer, reference, a `unique_ptr/shared_ptr` or something else. We shall define both of the dependent components as constructor parameters:

```c++
struct Car
{
    unique_ptr<Engine> engine;
    shared_ptr<ILogger> logger;

    Car(unique_ptr<Engine> engine, const shared_ptr<ILogger>& logger)
        : engine{move(engine)},
          logger{logger}
    {
        logger->Log("making a car");
    }

    friend ostream& operator<<(ostream& os, const Car& obj)
    {
        return os << "car with engine: " << *obj.engine;
    }
};
```

Now, you’re probably expecting to see `make_unique/make_shared` calls as we initialize the Car. But we won’t do any of that. Instead, we’ll use Boost.DI. First of all, we’ll define a binding that binds ILogger to ConsoleLogger; what this means is, basically, “any time someone asks for an ILogger give them a ConsoleLogger”:

```c++
auto injector = di::make_injector(
    di::bind<ILogger>().to<ConsoleLogger>()
);
```

And now that we’ve configured the injector, we can use it to create a car:

```c++
auto car = injector.create<shared_ptr<Car>>();
```

The preceding creates a `shared_ptr<Car>` that points to a *fully initialized* Car object, which is exactly what we wanted. The great thing about this approach is that, to change the type of logger being used, we can change it in a single place (the bind call) and every place where an ILogger appears can now be using some other logging component that we provide. This approach also helps us with unit testing, and allows us to use stubs (or the Null Object pattern) instead of mocks.

#### Time for Patterns!

With the understanding of the SOLID design principles, we are ready to take a look at the design patterns themselves. Strap yourselves in; it’s going to be a long (but hopefully not boring) ride!
