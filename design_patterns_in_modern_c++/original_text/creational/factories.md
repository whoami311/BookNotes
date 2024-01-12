# Factories

This chapter covers two GoF patterns at the same time: Factory Method and Abstract Factory. These patterns are closely related, so we’ll discuss them together.

## Scenario

Let’s begin with a motivating example. Support you want to store information about a Point in Cartesian space. So you go ahead and implement something like this:

```c++
struct Point
{
    Point(const float x, const float y)
        : x{x}, y{y} {}
    float x, y; // strictly Cartesian
};
```

So far, so good. But now, you also want to initialize the point using *polar* coordinates instead. You need another constructor with the signature:

```c++
Point(const float r, const float theta)
{
    x = r * cos(theta);
    y = r * sin(theta);
}
```

But unfortunately, you’ve already got a constructor with two floats, so you cannot have another one. What do you do? One approach is to introduce an enumeration:

```c++
enum class PointType
{
    cartesian,
    polar
};
```

and then add another parameter to the point constructor:

```c++
Point(float a, float b, PointType type = PointType::cartesian)
{
    if (type == PointType::cartesian)
    {
        x = a;
        y = b;
    }
    else
    {
        x = a * cos(b);
        y = a * sin(b);
    }
}
```

Notice how the names of the first two arguments were changed to a and b: we can no longer afford telling the user what coordinate system those values should come from. This is a clear loss of expressivity when compared with using x, y, rho, and theta to communicate intent.

Overall, our constructor design is usable, but ugly. Let’s see if we can improve it.

## Factory Method

The trouble with the constructor is that its name always matches the type. This means we cannot communicate any extra information in it, unlike in an ordinary function. Also, given the name is always the same, we cannot have two overloads one taking x,y and another taking r,theta.

So what can we do? Well, how about making the constructor protected and then exposing some static functions for creating new points?

```c++
struct Point
{
protected:
    Point(const float x, const float y)
        : x{x}, y{y} {}
public:
    static Point NewCartesian(float x, float y)
    {
        return { x, y };
    }
    static Point NewPolar(float r, float theta)
    {
        return { r * cos(theta), r * sin(theta) };
    }
    // other members here
};
```

Each of the above static functions is called a Factory Method. All it does is create a Point and return it, the advantages being that both the name of the method as well as the names of the arguments clearly communicate what kind of coordinates are required.

Now, to create a point, you simply write

```c++
auto p = Point::NewPolar(5, M_PI_4);
```

From the preceding, we can clealy surmise that we are creating a new point with polar coordinates *r = 5* and *theta = π/4*.

## Factory

Just like with Builder, we can take all the Point-creating functions out of Point into a separate class, a so-called Factory. First of all, we redefine the Point class:

```c++
struct Point
{
    float x, y;
    friend class PointFactory;
private:
    Point(float x, float y) : x(x), y(y) {}
};
```

Two things are worth noting here:

- The constructor of Point is private because we don’t want anyone calling it directly. This isn’t a strict requirement, but making it public creates a bit of an ambiguity, as it presents the user two different ways of constructing the object.
- Point declares PointFactory as a friend class. This is done deliberately so that the private constructor of Point isavailable to the factor—without this, the factory wouldn’t be able to instantiate the object! The implication here is that both of these types are being created at the same time, rather than the factory being created much later.

Now, we simply define our NewXxx() functions in a separate class called PointFactory:

```c++
struct PointFactory
{
    static Point NewCartesian(float x, float y)
    {
        return Point{ x, y };
    }
    static Point NewPolar(float r, float theta)
    {
        return Point{ r * cos(theta), r * sin(theta) };
    }
};
```

And that’s it—we now have a dedicated class specifically designed for creating Point instances, to be used as follows:

```c++
auto my_point = PointFactory::NewCartesian(3, 4);
```

## Inner Factory

An inner factory is simply a factory that is an inner class within the type it creates. To be fair, inner factories are typical artifacts of C#, Java, and other languages that lack the friend keyword, but there’s no reason one cannot have it in C++, too.

The reason why inner factories exist is because inner classes can access the outer class’ private members and, conversely, an outer class can access an inner class’ private members. This means that our Point class can also be defined as follows:

```c++
struct Point
{
private:
    Point(float x, float y) : x(x), y(y) {}

    struct PointFactory
    {
    private:
        PointFactory() {};
    public:
        static Point NewCartesian(float x, float y)
        {
            return {x, y};
        }
        static Point NewPolar(float r, float theta)
        {
            return { r * cos(theta), r * sin(theta) };
        }
    };
public:
    float x, y;
    static PointFactory Factory;
};
```

Okay, so what’s going on here? Well, we’ve stuck the factory right into the class the factory creates. This is convenient if a factory only works with one single type, and not so convenient if a factory relies on several types (and pretty much impossible if it needs their private members, too).

You’ll notice that I’m being sly here: the entire factory is inside a private block and, furthermore, its constructor is marked as private too. Essentially, even though we could expose this factory as Point::PointFactory, that’s quite a mouthful. Instead, I define a static member called Factory. This allows us to use the factory as

```c++
auto pp = Point::Factory.NewCartesian(2, 3);
```

If, for some reason, you don’t like mixing :: and ., you can, of course, change the code such that you use :: everywhere. The two says to do this are:

- Make the factory public, which lets you write

```c++
Point::PointFactory::NewXxx(...)`
```

- If you don’t like the word Point appearing twice in the preceding, you can typedef PointFactory Factory and then simply write Point::Factory::NewXxx(...). This is probably the most sensible syntax one can come up with. Or just call the inner factory Factory, which kind of solves the problem once and for all... unless you decide to factor it out later on.

The decision of whether or not to have an inner factory depends largely on how you like to organize your code. However, exposing the factory from the original object drastically improves the usability of the API. If I find a type called Point with a private constructor, how would I be able to tell that the class is meant to be used? Well, I wouldn’t unless Person:: gives me something meaningful in the code completion listing.

## Abstract Factory

So far, we’ve been looking at the construction of a single object. Sometimes, you might be involved in the creation of families of objects. This is actually a pretty rare case, so unlike Factory Method and just plain old Factory pattern, Abstract Factory is a pattern that only pops its head in complicated systems. We need to talk about it, regardless, primarily for historical reasons.

Here’s a simple scenario: suppose you are working at a café that serves tea and coffee. These two hot beverages are made through entirely different apparatus that we can both model as factories, of sorts. Tea and coffee can actually be served both hot or hold, but let’s focus on the hot variety. First of all, we can define what a HotDrink is:

```c++
struct HotDrink
{
    virtual void prepare(int volume) = 0;
};
```

The function prepare is what we would call to prepare a hot drink with a specific volume. For example, for a type Tea, it would be implemented as

```c++
struct Tea : HotDrink
{
    void prepare(int volume) override
    {
        cout << "Take tea bag, boil water, pour " << volume << "ml, add some lemon" << endl;
    }
};
```

And similarly for the Coffee type. At this point, we could write a hypothetical make_drink() function that would take the name of a drink and make that drink. Given a discrete set of cases, it can look rather tedious:

```c++
unique_ptr<HotDrink> make_drink(string type)
{
    unique_ptr<HotDrink> drink;
    if (type == "tea")
    {
        drink = make_unique<Tea>();
        drink->prepare(200);
    }
    else
    {
        drink = make_unique<Coffee>();
        drink->prepare(50);
    }
    return drink;
}
```

Now, remember, different drinks are made by different machinery. In our case, we’re interested in hot drinks, which we’ll model through the aptly named Hot-DrinkFactory:

```c++
struct HotDrinkFactory
{
    virtual unique_ptr<HotDrink> make() const = 0;
};
```

This type happens to be an Abstract Factory: it’s a factory with a specific interface, but it’s abstract, which means that even though it can feature as a function argument, for example, we would need concrete implementations to actually make the drinks. For example, in the case of making Coffee, we could write

```c++
struct CoffeeFactory : HotDrinkFactory
{
    unique_ptr<HotDrink> make() const override
    {
        return make_unique<Coffee>();
    }
}
```

And the same goes for TeaFactory, as before. Now, suppose we want to define a higher level interface for making different drinks, hot or cold. We could make a type called DrinkFactory that would itself contain references to the various factories that are available:

```c++
class DrinkFactory
{
    map<string, unique_ptr<HotDrinkFactory>> hot_factories;
public:
    DrinkFactory()
    {
        hot_factories["coffee"] = make_unique<CoffeeFactory>();
        hot_factories["tea"] = make_unique<TeaFactory>();
    }

    unique_ptr<HotDrink> make_drink(const string& name)
    {
        auto drink = hot_factories[name]->make();
        drink->prepare(200);    // oops!
        return drink;
    }
};
```

Here I’ve made an assumption that we want drinks dispensed based on their name rather than some integer or enum member. We simply make a map of strings and the associated factories: the actual factory type is HotDrinkFactory (our Abstract Factory), and we store them through smart pointers rather than directly (makes sense, because we want to prevent object slicing).

Now, when someone wants a drink, we find the relevant factory (think of a coffee shop assistant walking to the right machine), create the beverage, prepare exactly the volume required (I’ve set it to a constant in the preceding; feel free to promote it to a parameter) and then return the relevant drink. That’s all there is to it.

## Functional Factory

One last thing I wanted to mention: when we typically use the term *factory* we typically mean one of two things:

- A class that knows how to create objects
- A function that, when called, creates an object

The second option is not just a Factory Method in a classical sense. If someone passes in a std::function that returns a type T into some function, this is typically referred to as a Factory, not a Factory Method. This may seem a little weird, but when you consider the idea that a Method is synonymous with Member Function, it makes a bit more sense.

Luckily for us, functions can be stored in variables, which means that instead of just storing a pointer to the factory (as we did inDrinkFactory earlier), we can internalize the process of preparing exactly 200 ml of a liquid. This is done by switching from factories to simply using function blocks, for example:

```c++
class DrinkWithVolumeFactory
{
    map<string, function<unique_ptr<HotDrink>()>> factories;
public:
    DrinkWithVolumeFactory()
    {
        factories["tea"] = [] {
            auto tea = make_unique<Tea>();
            tea->prepare(200);
            return tea;
        };  // similar for Coffee
    }
};
```

Of course, having taken this approach, we are now reduced to calling the stored factory directly, that is:

```c++
inline unique_ptr<HotDrink>
DrinkWithVolumeFactory::make_drink(const string& name)
{
    return factories[name]();
}
```

And this can then be used as before.

## Summary

Let’s recap the terminology:

- A factory method is a class member that acts as a way of creating object. It typically replaces a constructor.
- A factory is typically a separate class that knows how to construct objects, though if you pass a function (as in std::function or similar) that constructs objects, this argument is also called a factory.
- An abstract factory is, as its name suggests, an abstract class that can be inherited by concrete classes that offer a family of types. Abstract factories are rare in the wild.

A factory has several critical advantages over a constructor call, namely:

- A factory can say no, meaning that instead of actually returning an object it can return, for example, a nullptr.
- Naming is better and unconstrained, unlike constructor name.
- A single factory can make objects of many different types.
- A factory can exhibit polymorphic behavior, instantiating a class and returning it through its base class’ reference or pointer.
- A factory can implement caching and other storage optimizations; it is also a natural choice for approaches such as pooling or the Singleton pattern (more on this in Chapter 5).

Factory is different from Builder in that, with a Factory, you typically create an object in one go, whereas with Builder, you construct the object piecewise by providing information in parts.
