# Singleton

The Singleton is the most hated design pattern in the (rather limited) history of design patterns. Just stating that, however, doesn’t mean you shouldn’t use the singleton: a toilet brush is not the most pleasant device either, but sometimes it’s simply necessary.

The Singleton design pattern grew out of a very simple idea that you should only have one instance of a particular component in your application. For example, a component that loads a database into memory and offers a read-only interface is a prime candidate for a Singleton, since it really doesn’t make sense to waste memory storing several identical datasets. In fact, your application might have constraints such that two or more instances of the database simply won’t fit into memory, or will result in such a lack of memory as to cause the program to malfunction.

## Singleton as Global Object

The naïve approach to this problem is to simply agree that we are not going to instantiate this object ever, for example:

```c++
struct Database
{
    /**
    * \brief Please do not create more than one instance.
    */
    Database() {}
};
```

Now, the problem with this approach, apart from the fact that your developer colleagues might simply ignore the advice, is that objects can be created in stealthy ways where the call to the constructor isn’t immediately obvious. This can be anything—copy constructor/assignment, a `make_unique()` call, or the use of an inversion of control (IoC) container.

The most obvious idea that comes to mind is to offer a single, static global object:

```c++
static Database database{};
```

The trouble with global static objects is that their initialization order in different compilation units is undefined. This can lead to nasty effects, like one global object referring to another when the latter hasn’t yet been initialized. There’s also the issue of discoverability: how does the client know that a global variable exists? Discovering classes is somewhat easier becaue **Go to Type** gives a much more reduced set then autocompletion after ::.

One way to mitigate this is to offer a global (or indeed, member) *function* that exposes the necessary object:

```c++
Database& get_database()
{
    static Database database;
    return database;
}
```

This function can be called to get a reference to the database. You should be aware, however, that thread safety for the preceding is only guaranteed since C++11, and you should check whether your compiler is actually prepated to insert locks to prevent concurrent access while the static object is initializing.

Of course, it’s very easy for this scenario to go pear shaped: if Database decides to use some other, similarly exposed, singleton in its destructor, the program is likely to blow up. This raises more of a philosophical point: is it OK for singletons to refer to other singletons?

## Classic Implementation

One aspect of the preceding implementations that has been completely neglected is the prevention of the construction of additional objects. Having a global static Database doesn’t really prevent anyone from making another instance.

We can easily turn life sour for those interested in making more than one instance of an object: simply put a static counter right in the constructor and throw if the value is ever incremented:

```c++
struct Database
{
    Database()
    {
        static int instance_count{ 0 };
        if (++instance_count > 1)
            throw std::exception("Cannot make >1 database!");
    }
};
```

This is a particularly hostile approach to the problem: even though it prevents the creation of more than one instance by throwing an exception, it fails to *communicate* the fact that we don’t want anyone calling the constructor more than once.

The only way to prevent explicit construction of Database is to once again make its constructor private and introduce the aforementioned function as a *member* function to return the one and only instance:

```c++
struct Database
{
protected:
    Database() { /* do what you need to do */ }
public:
    static Database& get()
    {
        // thread-safe in C++11
        static Database database;
        return database;
    }
    Database(Database const&) = delete;
    Database(Database&&) = delete;
    Database& operator=(Database const&) = delete;
    Database& operator=(Database &&) = delete;
};
```

Note how we completely removed any possibility of creating Database instances by hiding the constructor and deleting copy/move constructor/assignment operators.

In pre-C++11 days, you would simply make the copy constructor/assignment private to achieve roughly the same purpose. As an alternative to doing this by hand, you might want to check out boost::noncopyable, a class that you can inherit that adds roughly the same definitions in terms of hiding the members… except it doesn’t affect move constructor/assignment.

I reiterate, once again, that if database depends on other static or global variables, using them in its destructor is not safe, as destruction order for these objects is not deterministic, and you might actually be calling objects that have already been destroyed.

Finally, in a particularly nasty trick, you can impletment get() as a heap allocation (so that only the pointer, not the entire object, is static).

```c++
static Database& get() {
    static Database* database = new Database();
    return *database;
}
```

The preceding implementation relies on the assumption that Database lives until the end of the program and the use of a pointer instead of a reference ensures that a destructor, even if you make one (which, if you do, would have to be public), is never called. And no, the preceding code doesn’t cause a memory leak.

### Thread Safety

As I’ve already mentioned, initialization of a singleton in the manner listed previously is thread safe since C++11, meaning that if two threads were to simultaneously call get(), we wouldn’t run into a situation where the database would be created twice.

Prior to C++11, you would construct the singleton using an approach called double-checked locking. A typical implementation would look like this:

```c++
struct Database
{
    // same members as before, but then...
    static Database& instance();
private:
    static boost::atomic<Database*> instance;
    static boost::mutex mtx;
};

Database& Database::instance()
{
    Database* db = instance.load(boost::memory_order_consume);
    if (!db)
    {
        boost::mutex::scoped_lock lock(mtx);
        db = instance.load(boost::memory_order_consume);
        if (!db)
        {
            db = new Database();
            instance.store(db, boost::memory_order_release);
        }
    }
}
```

Since this book is concerned with Modern C++, we won’t dwell on this approach any further.

## The Trouble with Singleton

Let’s suppose that our database contains a list of capital cities and their populations. The interface that our singleton database is going to conform to is:

```c++
class Database
{
public:
    virtual int get_population(const std::string& name) = 0;
};
```

We have a single member function that gets us the population for a given city. Now, let us suppose that this interface is adopted by a concrete implementation called `SingletonDatabase` that implements the singleton the same way as we’ve done before:

```c++
class SingletonDatabase : public Database
{
    SingletonDatabase() { /* read data from database */ }
    std::map<std::string, int> capitals;
public:
    SingletonDatabase(SingletonDatabase const&) = delete;
    void operator=(SingletonDatabase const&) = delete;

    static SingletonDatabase* get()
    {
        static SingletonDatabase db;
        return db;
    }

    int get_population(const std::string& name) override
    {
        return capitals[name];
    }
};
```

As we noted, the real problem with singletons like the preceding one is their use in other components. Here’s what I mean: suppose that, on the basis of the preceding example, we build a component for calculating the sum total population of several different cities:

```c++
struct SingletonRecordFinder
{
    int total_population(std::vector<std::string> names)
    {
        int result = 0;
        for (auto& name : names)
            result += SingletonDatabase::get().get_population(name);
        return result;
    }
};
```

The trouble is that SingletonRecordFinder is now firmly dependent on SingletonDatabase. This presents an issue for testing: if we want to check that SingletonRecordFinder works correctly, we need to use data from the actual database, that is:

```c++
TEST(RecordFinderTests, SingletonTotalPopulationTest)
{
    SingletonRecordFinder rf;
    std::vector<std::string> names{ "Seoul", "Mexico City" };
    int tp = rf.total_population(names);
    EXPECT_EQ(17500000 + 17400000, tp);
}
```

But what if we don’t want to use an actual database for a test? What if we want to use some other dummy component instead? Well, in our current design, this is impossible, and it is precisely this inflexibility that is the Singeton’s downfall.

So, what can we do? Well, for one, we need to stop depending on Singleton-Database explicitly. Since all we need is something implementing the Database interface, we can create a new `ConfigurableRecordFinder` that lets us configure where the data comes from:

```c++
struct ConfigurableRecordFinder
{
    explicit ConfigurableRecordFinder(Database& db)
        : db{db} {}

    int total_population(std::vector<std::string> names)
    {
        int result = 0;
        for (auto& name : names)
            result += db.get_population(name);
        return result;
    }

    Database& db;
}
```

We now use the db reference instead of using the singleton explicitly. This lets us make a dummy database specifically for testing the record finder:

```c++
class DummyDatabase : public Database
{
    std::map<std::string, int> capitals;
public:
    DummyDatabase()
    {
        capitals["alpha"] = 1;
        capitals["beta"] = 2;
        capitals["gamma"] = 3;
    }

    int get_population(const std::string& name) override {
        return capitals[name];
    }
};
```

And now, we can rewrite our unit test to take advantage of this `DummyDatabase`:

```c++
TEST(RecordFinderTests, DummyTotalPopulationTest)
{
    DummyDatabase db{};
    ConfigurableRecordFinder rf{ db };
    EXPECT_EQ(4, rf.total_population(std::vector<std::string>{"alpha", "gamma"}));
}
```

This test is more robust because if data changes in the actual database, we won’t have to adjust our unit test values—the dummy data stays the same.

## Singletons and Inversion of Control

The approach with explicitly making a component a singleton is distinctly invasive, and a decision to stop treating the class as a Singleton down the line will end up particularly costly. An alternative solution is to adopt a convention where, instead of directly enforcing the lifetime of a class, this function is outsourced to an IoC container.

Here’s what defining a singleton component looks like when using the Boost.DI dependency injection framework:

```c++
auto injector = di::make_injector(
    di::bind<IFoo>.to<Foo>.in(di::singleton),
    // other configuration steps here
);
```

In the preceding, I use the first letter I in a type name to indicate an interface type. Essentially, what the di::bind line says is that, whenever we need a component that has a member of type IFoo, we initialize that component with a singleton instance of Foo.

According to many people, using a singleton in a DI container is the only socially acceptable use of a singleton. At least with this approach, if you need to replace a singleton object with something else, you can do it in one central place: the container configuration code. An added benefit is that you won’t have to implement any singleton logic yourself, which prevents possible errors. Oh, and did I mention that Boost.DI is thread safe?

## Monostate

Monostate is a variation on the Singleton pattern. It is a class that behaves like a singleton while appearing as an ordinary class.

```c++
class Printer
{
    static int id;
public:
    int get_id() const { return id; }
    void set_id(int value) { id = value; }
};
```

Can you see what’s happening here? The class appears as an ordinary class with getters and setters, but they actually work on static data!

This might seem like a really neat trick: you let people instantiate Printer but they all refer to the same data. However, how are users supposed to know this? A user will happily instantiate two printers, assign them different ids, and be very surprised when both of them are identical!

The Monostate approach works to some degree and has a couple of advantages. For example, it is easy to inherit, it can leverage polymorphism, and its lifetime is reasonbly well defined (but then again, you might not always wish it so). Its greatest advantage is that you can take an existing object that’s already used throughout the system, patch it up to behave in a Monostate way, and provided your system works fine with the nonplurality of object intances, you’ve got yourself a Singleton-like implementation with no extra code needing to be rewritten.

The disadvantages are obvious, too: it is an intrusive approach (converting an ordinary object to a Monostate is not easy), and its use of static members means it always takes up space, even when it’s not needed. Ultimately, Monostate’s greatest downfall is that it makes a very optimistic assumption that the class fields are always exposed through getters and setters. If they are being accessed directly, your refactoring is almost doomed to fail.

## Summary

Singletons aren’t totally evil but, when used carelessly, they’ll mess up the testability and refactorability of your application. If you really must use a singleton, try avoiding using it directly (as in, writing `SomeComponent. getInstance().foo()`) and instead keep specifying it as a dependency (e.g., a constructor argument) where all dependencies are satisfied from a single location in your application (e.g., an inversion of *control* container).