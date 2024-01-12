# Factories

本章同时介绍两种GoF模式：工厂方法和抽象工厂。这两种模式密切相关，因此我们将把它们一起讨论。

## Scenario

让我们从一个激励性的例子开始。支持您存储 笛卡尔空间中一个点的信息。因此，您可以实现这样的功能：

```c++
struct Point
{
    Point(const float x, const float y)
        : x{x}, y{y} {}
    float x, y; // strictly Cartesian
};
```

到目前为止，一切顺利。但现在，你还想用*极*坐标来初始化点。您需要另一个构造函数，其签名为

```c++
Point(const float r, const float theta)
{
    x = r * cos(theta);
    y = r * sin(theta);
}
```

但不幸的是，你已经有了一个包含两个浮点的构造函数，所以不能再有另一个了。怎么办呢？一种方法是引入一个枚举：

```c++
enum class PointType
{
    cartesian,
    polar
};
```

然后在点构造函数中添加另一个参数：

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

请注意，前两个参数的名称被改为 a 和 b：我们不能再告诉用户这些值应该来自哪个坐标系了。与使用 x、y、rho 和 theta 来传达意图相比，这明显丧失了表达能力。

总的来说，我们的构造函数设计可用，但很难看。让我们看看能否改进它。

## Factory Method

构造函数的问题在于，它的名称总是与类型相匹配。这意味着我们无法在构造函数中传递任何额外信息，这与普通函数不同。此外，由于名称总是相同的，我们无法使用两个重载函数，一个使用 x,y，另一个使用 r,theta。

那么我们能做些什么呢？那么，让构造函数受保护，然后公开一些用于创建新点的静态函数如何？

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

上述每个静态函数都称为工厂方法（Factory Method）。它所做的就是创建一个点并返回它，其优点是方法的名称和参数的名称都清楚地表达了需要什么样的坐标。

现在，要创建一个点，只需写下

```c++
auto p = Point::NewPolar(5, M_PI_4);
```

从上文我们可以清楚地推测出，我们正在创建一个极坐标为 *r = 5* 和 *theta = π/4* 的新点。

## Factory

就像使用 Builder 一样，我们可以将所有创建 Point 的功能从 Point 中移出，放入一个单独的类，即所谓的 Factory。首先，我们要重新定义 Point 类：

```c++
struct Point
{
    float x, y;
    friend class PointFactory;
private:
    Point(float x, float y) : x(x), y(y) {}
};
```

这里有两点值得注意：

- Point 的构造函数是私有的，因为我们不希望任何人直接调用它。这并不是一个严格的要求，但将它设为公有会产生一些歧义，因为它向用户提供了两种不同的构造对象的方法。
- Point 将 PointFactory 声明为友元类。这样做是故意的，这样 Point 的私有构造函数就可以被factor使用--如果没有这个构造函数，工厂就无法实例化对象！这里的含义是，这两个类型是同时创建的，而不是在很后面才创建工厂。

现在，我们只需在一个名为 PointFactory 的单独类中定义 NewXxx() 函数即可：

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

就是这样，我们现在有了一个专门用于创建点实例的专用类，使用方法如下：

```c++
auto my_point = PointFactory::NewCartesian(3, 4);
```

## Inner Factory

所谓内部工厂，就是在其创建的类型中包含一个内部类的工厂。平心而论，内部工厂是 C#、Java 和其他缺少 friend 关键字的语言的典型产物，但 C++ 中也可以有。

内部工厂之所以存在，是因为内部类可以访问外部类的私有成员，反之，外部类也可以访问内部类的私有成员。这意味着我们的 Point 类也可以定义如下：

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

好吧，这里发生了什么？我们把工厂直接植入了工厂创建的类中。如果工厂只与一种类型打交道，这就很方便了，但如果工厂依赖于多种类型，就不那么方便了（如果还需要它们的私有成员，那就几乎不可能了）。

你会发现我在这里很狡猾：整个工厂都在私有代码块中，而且它的构造函数也被标记为私有。从根本上说，尽管我们可以把这个工厂公开为 Point::PointFactory，但这实在是太拗口了。相反，我定义了一个名为 Factory 的静态成员。这样，我们就可以将工厂作为

```c++
auto pp = Point::Factory.NewCartesian(2, 3);
```

如果出于某种原因，您不喜欢混合使用::和...，您当然可以修改代码，在所有地方都使用::。有两种方法可以做到这一点：

- 将工厂设为公共工厂，这样就可以编写

```c++
Point::PointFactory::NewXxx(...)`
```

- 如果不喜欢 Point 这个词在前面出现两次，可以类型化 PointFactory Factory，然后简单地写成 Point::Factory::NewXxx(...)。这可能是最合理的语法了。或者直接调用内部工厂 Factory，这样就可以一劳永逸地解决这个问题......除非你决定以后再把它考虑进去。

决定是否使用内部工厂主要取决于您喜欢如何组织代码。不过，从原始对象中公开工厂可以大大提高 API 的可用性。 如果我发现一个名为 Point 的类型有一个私有构造函数，我怎么才能知道这个类是要被使用的呢？除非 Person:: 在代码完成列表中给我一些有意义的信息，否则我是不会知道的。

## Abstract Factory

到目前为止，我们已经了解了单个物体的构造。有时，你可能需要创建对象家族。这实际上是一种非常罕见的情况，因此，与工厂方法（Factory Method）和普通的工厂模式（Factory pattern）不同，抽象工厂模式（Abstract Factory）只有在复杂的系统中才会出现。无论如何，我们都需要谈谈它，这主要是出于历史原因。

这里有一个简单的场景：假设你在一家提供茶和咖啡的咖啡馆工作。这两种热饮是通过完全不同的设备制作的，我们都可以把它们当作某种工厂来模拟。实际上，茶和咖啡既可以热饮，也可以保温，但我们重点讨论热饮。首先，我们可以定义什么是热饮：

```c++
struct HotDrink
{
    virtual void prepare(int volume) = 0;
};
```

我们调用函数 prepare 来准备一杯特定容量的热饮。例如，对于茶叶类型，它的实现方式是

```c++
struct Tea : HotDrink
{
    void prepare(int volume) override
    {
        cout << "Take tea bag, boil water, pour " << volume << "ml, add some lemon" << endl;
    }
};
```

咖啡类型也是如此。此时，我们可以编写一个假想的 make_drink() 函数，该函数将接收饮料名称并制作该饮料。在离散的情况下，这看起来可能相当繁琐：

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

请记住，不同的饮料由不同的机器制造。在我们的案例中，我们感兴趣的是热饮，我们将通过恰如其分地命名为 "热饮工厂"（Hot-DrinkFactory）来对其进行建模：

```c++
struct HotDrinkFactory
{
    virtual unique_ptr<HotDrink> make() const = 0;
};
```

这种类型恰好是抽象工厂：它是一个具有特定接口的工厂，但它是抽象的，这意味着即使它可以作为函数参数，我们也需要具体的实现来真正制作饮料。例如，在制作咖啡的情况下，我们可以编写

```c++
struct CoffeeFactory : HotDrinkFactory
{
    unique_ptr<HotDrink> make() const override
    {
        return make_unique<Coffee>();
    }
}
```

TeaFactory 也是如此。现在，假设我们想定义一个更高级别的界面，用于制作不同的冷热饮品。我们可以创建一个名为 DrinkFactory 的类型，它本身将包含对各种可用工厂的引用：

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

在这里，我假设我们希望根据饮料的名称而不是某个整数或枚举成员来分配饮料。我们只需创建一个字符串和相关工厂的映射：实际的工厂类型是 HotDrinkFactory（我们的抽象工厂），我们通过智能指针而不是直接存储它们（这是合理的，因为我们要防止对象切片）。

现在，当有人想喝饮料时，我们就会找到相关的工厂（想象一下咖啡店的店员走到正确的机器前），制作饮料，准确地准备好所需的体积（我在前面将其设置为常数；您可以将其提升为参数），然后返回相关的饮料。仅此而已。

## Functional Factory

最后我想说的是：当我们使用 "工厂 "一词时，我们通常指的是两种情况中的一种：

- 一个知道如何创建对象的类
- 调用时创建对象的函数

第二种选择并不仅仅是经典意义上的工厂方法。如果有人将一个返回 T 类型的 std::function 传递给某个函数，这通常被称为 Factory，而不是 Factory Method。这似乎有点奇怪，但如果考虑到 "方法"（Method）与 "成员函数"（Member Function）是同义词的概念，这就说得通了。

对我们来说幸运的是，函数可以存储在变量中，这意味着我们可以将准备 200 毫升液体的过程内部化，而不只是存储一个指向工厂的指针（就像我们之前在 DrinkFactory 中所做的）。例如，我们可以将工厂转换为简单的功能块：

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

当然，采用这种方法后，我们现在只能直接调用存储工厂，即

```c++
inline unique_ptr<HotDrink>
DrinkWithVolumeFactory::make_drink(const string& name)
{
    return factories[name]();
}
```

然后就可以像之前一样使用了。

## Summary

让我们回顾一下术语：

- 工厂方法是类成员创建对象的一种方法。它通常取代构造函数。
- 工厂通常是一个知道如何构造对象的独立类，不过如果你传递一个构造对象的函数（如 std::function 或类似函数），这个参数也被称为工厂。
- 抽象工厂，顾名思义，是一个抽象类，可以被提供一系列类型的具体类继承。抽象工厂在实际应用中并不多见。

与构造函数调用相比，工厂有几个关键优势，即

- 工厂可以说 "不"，也就是说，它可以不实际返回一个对象，而是返回一个 nullptr。
- 与构造函数名称不同的是，它的命名更好且不受限制。
- 一个工厂可以生产多种不同类型的对象。
- 工厂可以表现出多态行为，实例化一个类，并通过基类的引用或指针返回该类。
- 工厂可以实现缓存和其他存储优化；它也是池模式或单件模式等方法的自然选择（更多内容请参见第 5 章）。

工厂 "与 "生成器 "的不同之处在于，使用 "工厂 "时，通常是一次性创建一个对象，而使用 "生成器 "时，则是通过分段提供信息来构建对象。
