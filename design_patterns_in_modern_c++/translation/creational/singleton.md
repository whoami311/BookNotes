# Singleton

在（相当有限的）设计模式历史上，单例是最令人憎恨的设计模式。然而，这并不意味着你不应该使用单例：马桶刷也不是最讨人喜欢的设备，但有时它就是必要的。

单例设计模式源于一个非常简单的想法：在应用程序中，某个组件只能有一个实例。例如，将数据库加载到内存中并提供只读接口的组件就是单例的最佳候选，因为浪费内存来存储多个相同的数据集是毫无意义的。事实上，你的应用程序可能有一些限制，比如两个或更多的数据库实例根本无法装入内存，或者会导致内存不足，从而导致程序运行故障。

## Singleton as Global Object

解决这个问题的最简单的方法是，例如，我们同意永远不实例化这个对象：

```c++
struct Database
{
    /**
    * \brief Please do not create more than one instance.
    */
    Database() {}
};
```

现在，这种方法的问题除了你的开发同事可能会忽略这些建议之外，还在于对象可以通过隐蔽的方式创建，在这种情况下，对构造函数的调用并不明显。这可以是任何方式————复制构造函数/分配、调用 "make_unique() "或使用反转控制（IoC）容器。

最明显的想法是提供一个单一的静态全局对象：

```c++
static Database database{};
```

全局静态对象的问题在于，它们在不同编译单元中的初始化顺序是不确定的。这可能会导致令人讨厌的后果，比如一个全局对象在尚未初始化的情况下引用另一个全局对象。还有一个可发现性问题：客户端如何知道全局变量的存在？发现类在某种程度上比较容易，因为 **Go to Type** 提供的类集比在 :: 后自动完成的要少得多。

缓解这一问题的方法之一是提供一个全局（或成员）*function*，公开必要的对象：

```c++
Database& get_database()
{
    static Database database;
    return database;
}
```

调用该函数可以获取数据库的引用。不过，您应该注意，只有 C++11 才保证了前述函数的线程安全，您应该检查编译器是否已经准备好在静态对象初始化时插入锁以防止并发访问。

当然，这种情况很容易出问题：如果数据库决定在其析构函数中使用其他类似的单例，程序很可能会崩溃。这就提出了一个哲学问题：单例可以引用其他单例吗？

## Classic Implementation

在前面的实现中，有一个方面完全被忽视了，那就是防止创建其他对象。拥有一个全局静态数据库并不能真正阻止任何人创建另一个实例。

对于那些想创建不止一个对象实例的人来说，我们可以轻松地让生活变得不那么美好：只需在构造函数中加入一个静态计数器，并在值递增时抛出即可：

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

这是一种特别不利于解决问题的方法：尽管它通过抛出异常来防止创建多个实例，但它未能 *communicate* 一个事实，即我们不希望任何人多次调用构造函数。

防止显式构造数据库的唯一方法是再次将其构造函数设为私有，并将上述函数作为*成员*函数引入，以返回唯一的实例：

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

请注意，我们通过隐藏构造函数和删除复制/移动构造函数/赋值操作符，完全消除了创建数据库实例的可能性。

在 C++ 11 之前的时代，您只需将复制构造函数/赋值设置为私有即可实现大致相同的目的。作为手工操作的替代方法，你可能想看看 boost::noncopyable，这是一个你可以继承的类，它在隐藏成员方面添加了大致相同的定义......只是它不影响移动构造函数/赋值。

我再次重申，如果数据库依赖于其他静态变量或全局变量，那么在析构函数中使用这些变量是不安全的，因为这些对象的销毁顺序并不是确定的，你可能会调用已经销毁的对象。

最后，有一个特别讨厌的技巧，就是将 get() 作为堆分配来实现（这样只有指针而不是整个对象是静态的）。

```c++
static Database& get() {
    static Database* database = new Database();
    return *database;
}
```

前面的实现依赖于这样一个假设，即数据库会一直存活到程序结束，而使用指针而不是引用可以确保析构函数即使被调用（如果被调用，也必须是公共的），也不会被调用。不，前面的代码不会导致内存泄漏。

### Thread Safety

正如我前面提到的，从 C++11 开始，以前面列出的方式初始化单例是线程安全的，这意味着如果两个线程同时调用 get()，我们不会遇到数据库被创建两次的情况。

在 C++11 之前，您可以使用一种称为双重检查锁定的方法来构造单例。典型的实现如下

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

由于本书涉及的是现代 C++，我们将不再赘述这种方法。

## The Trouble with Singleton

假设我们的数据库包含一个首都及其人口的列表。我们的单例数据库要遵从的接口是

```c++
class Database
{
public:
    virtual int get_population(const std::string& name) = 0;
};
```

我们有一个成员函数，它可以获取给定城市的人口数。现在，让我们假设这个接口被一个名为 `SingletonDatabase` 的具体实现所采用，该具体实现与我们之前所做的相同：

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

正如我们所指出的，类似前述的单例的真正问题在于它们在其他组件中的使用。我的意思是：假设在前面例子的基础上，我们建立了一个计算多个不同城市人口总数的组件：

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

问题在于，`SingletonRecordFinder` 现在完全依赖于 `SingletonDatabase`。这就给测试带来了一个问题：如果我们要检查 `SingletonRecordFinder` 是否正常工作，就必须使用实际数据库中的数据：

```c++
TEST(RecordFinderTests, SingletonTotalPopulationTest)
{
    SingletonRecordFinder rf;
    std::vector<std::string> names{ "Seoul", "Mexico City" };
    int tp = rf.total_population(names);
    EXPECT_EQ(17500000 + 17400000, tp);
}
```

但如果我们不想在测试中使用实际数据库呢？如果我们想使用其他虚拟组件呢？在我们目前的设计中，这是不可能的，而正是这种不灵活性成为了 Singeton 的弊端。

那么，我们能做些什么呢？首先，我们不能再明确依赖 Singleton-Database 了。既然我们需要的只是实现数据库接口的东西，那么我们就可以创建一个新的 `ConfigurableRecordFinder` 来让我们配置数据的来源：

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

我们现在使用 db 引用，而不是显式地使用单例。这样我们就可以创建一个虚拟数据库，专门用于测试记录查找器：

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

现在，我们可以重写单元测试以利用这个 `DummyDatabase` ：

```c++
TEST(RecordFinderTests, DummyTotalPopulationTest)
{
    DummyDatabase db{};
    ConfigurableRecordFinder rf{ db };
    EXPECT_EQ(4, rf.total_population(std::vector<std::string>{"alpha", "gamma"}));
}
```

这种测试更稳健，因为如果实际数据库中的数据发生变化，我们无需调整单元测试值————假数据保持不变。

## Singletons and Inversion of Control

显式地将组件设为单例的方法具有明显的侵入性，如果决定在下一步停止将类作为单例处理，最终将付出特别高的代价。另一种解决方案是采用一种惯例，即不直接强制执行类的生命周期，而是将此功能外包给 IoC 容器。

下面是使用 Boost.DI 依赖注入框架时定义单例组件的情况：

```c++
auto injector = di::make_injector(
    di::bind<IFoo>.to<Foo>.in(di::singleton),
    // other configuration steps here
);
```

在前文中，我使用类型名称中的第一个字母 I 来表示接口类型。从本质上讲，di::bind 这一行的意思是，当我们需要一个拥有 IFoo 类型成员的组件时，我们就用 Foo 的单例来初始化该组件。

许多人认为，在 DI 容器中使用单例是社会上唯一可以接受的单例用法。至少使用这种方法，如果需要用其他东西替换单例对象，可以在一个中心位置完成：容器配置代码。这样做的另一个好处是，你不必亲自实现任何单例逻辑，从而避免了可能出现的错误。哦，我有没有提到 Boost.DI 是线程安全的？

## Monostate

Monostate 是单例模式的一种变体。它是一种行为类似于单例的类，同时又以普通类的形式出现。

```c++
class Printer
{
    static int id;
public:
    int get_id() const { return id; }
    void set_id(int value) { id = value; }
};
```

你能看出这里发生了什么吗？这个类看起来就像一个普通的类，有getter和setter，但它们实际上是对静态数据起作用的！

这似乎是一个非常巧妙的技巧：你可以让人们实例化 Printer，但它们都引用相同的数据。然而，用户如何知道这一点呢？用户会很高兴地实例化两台打印机，为它们分配不同的 ID，然后当两台打印机完全相同时，他们会非常惊讶！

Monostate方法在某种程度上是可行的，而且有几个优点。例如，它易于继承，可以利用多态性，而且它的生命周期定义得非常合理（但话说回来，你可能并不总是希望如此）。它最大的优点是，你可以使用一个已经在整个系统中使用的现有对象，将其修补成 Monostate 的行为方式，只要你的系统能很好地处理对象实例的非多重性，你就可以得到一个Singleton-like的实现，而不需要重写额外的代码。

缺点也很明显：这是一种侵入式方法（将普通对象转换为 Monostate 并不容易），而且静态成员的使用意味着它总是占用空间，即使在不需要的时候也是如此。归根结底，Monostate 最大的缺陷在于它做了一个非常乐观的假设，即类的字段总是通过 getter 和 setter 暴露出来。如果它们被直接访问，你的重构几乎注定要失败。

## Summary

单例并不完全是邪恶的，但如果使用不慎，就会破坏应用程序的可测试性和可重构性。如果您真的必须使用单例，请尽量避免直接使用它（如编写 `SomeComponent. getInstance().foo()` ），而应将其指定为依赖项（如构造函数参数），在应用程序中的一个位置（如 *control* 容器的反转）满足所有依赖项。
