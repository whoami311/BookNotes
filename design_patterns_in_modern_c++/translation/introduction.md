# Design Patterns in Modern C++

## Introduction

设计模式这个话题听起来枯燥乏味，而且老实说，几乎在所有能想象到的编程语言中都被扼杀了，包括JavaScript这样的编程语言，它甚至都不是完全的面向对象编程!那为什么还要写一本关于它的书呢?

我猜这本书存在的主要原因是c++又一次伟大了。在经历了很长一段时间的停滞之后，它现在正在进化、成长，尽管它必须与C语言的向后兼容性作斗争，但好事正在发生，尽管不是以我们都希望的速度。(除了其他事情之外，我还在查看模块。)

现在，说到设计模式，我们不应该忘记最初出版的设计模式书1是用c++和Smalltalk编写的示例。从那时起，许多编程语言都将设计模式直接纳入到语言中:例如，c#直接将观察者模式与其内置的事件支持(以及相应的event关键字)结合起来。c++没有做同样的事情，至少在语法层面上没有。也就是说，std::function等类型的引入确实让很多编程场景变得更加简单。

设计模式也是一种有趣的探索，它探索了一个问题如何通过不同程度的技术复杂性和不同类型的权衡来解决。有些模式或多或少是必不可少的，不可避免的，而其他模式则更多地是出于科学好奇心(但无论如何，我会在本书中讨论，因为我是一个完美主义者)。

读者应该意识到，对某些问题的全面解决方案(例如，观察者模式)通常会导致过度工程，也就是说，创建的结构比大多数典型场景所需要的结构复杂得多。虽然过度工程很有趣(嘿，你可以真正解决问题并给你的同事留下深刻印象)，但通常是不可行的。

### Preliminaries

#### Who This Book Is For

本书旨在成为经典 GoF 书籍的现代升级版，特别针对 C++ 编程语言。我的意思是，你们当中有多少人在编写 Smalltalk？我猜不会很多。

本书的目标是研究我们如何将现代C++（目前最新的C++版本）应用于经典设计模式的实现。同时，本书也试图充实任何可能对 C++ 开发人员有用的新模式和新方法。

最后，在某些地方，本书简直就是现代 C++ 的技术演示，展示了现代 C++ 的一些最新特性（如例行程序）如何使难题变得更容易解决。

#### On Code Examples

本书中的示例都适合用于生产，但为了提高可读性，还是做了一些简化：

- 很多时候，你会发现我使用 struct 而不是 class 来避免在太多地方使用 public 关键字。
- 我会避免使用`std::`前缀，因为它会影响可读性，尤其是在代码密度较高的地方。如果我使用`string`，你可以肯定我指的是`std::string`。
- 我会避免添加虚拟析构函数，但在现实生活中，添加虚拟析构函数可能是有意义的。
- 在极少数情况下，我会通过值来创建和传递参数，以避免`shared_ptr/make_shared`等的泛滥。智能指针增加了复杂性的另一个层次，将其集成到本书介绍的设计模式中将留给读者一个练习。
- 我有时会省略一些代码元素，这些元素对于完成一个类型的功能是必要的（例如，移动构造函数），因为它们占用了太多的空间。
- 在很多情况下，我会省略`const`，而在正常情况下，这其实是有意义的。const-correctness常常会导致API表面的分裂和加倍，这在书本格式中是行不通的。

需要注意的是，大多数示例都使用了现代 C++（C++11、14、17 及更高版本），而且一般都使用了开发人员可用的最新 C++ 语言特性。例如，当 C++ 14 允许我们自动推断返回类型时，你就不会看到很多以`-> decltype(...)`结尾的函数签名了。这些示例都不针对特定的编译器，但如果某些功能在你选择的编译器中无法运行，你就需要找到变通方法。

在某些时候，我会参考其他编程语言，如 C# 或 Kotlin。注意其他语言的设计者是如何实现特定功能的，有时会很有趣。C++ 从其他语言中借鉴普遍可用的想法并不陌生：例如，在变量声明和返回类型中引入自动推理和类型推理，这在许多其他语言中都存在。

#### Prince

数字盗版是生活中无法回避的事实。全新一代人正在成长，他们从未购买过电影或书籍，即使是这本书。对此我们无能为力。我唯一能说的是，如果你盗版了这本书，你可能读到的不是最新版本。

在线数字出版的乐趣在于，我可以根据 C++ 的新版本和我所做的更多研究来更新本书。因此，如果你花钱买了这本书，那么在未来新版本的 C++ 语言和标准库发布时，你将获得免费更新。如果没有......哦，好吧。

### Important Concepts

在开始之前，我想简单提一下 C++ 世界的一些关键概念，这些概念将在本书中提及。

#### Curiously Recurring Template Pattern

嘿，这显然是一种模式！我不知道它是否有资格被列为一种独立的设计模式，但它肯定是 C++ 世界中的一种模式。从根本上说，这个想法很简单：继承者将自己作为模板参数传递给基类：

```c++
struct Foo : SomeBase<Foo>
{
    ...
}
```

现在，你可能想知道为什么要这么做？其中一个原因是为了能够在基类实现中访问类型化的 this 指针。

例如，假设 SomeBase 的每个继承者都实现了迭代所需的 `begin()/end()` 对。你如何才能迭代 `SomeBase` 成员内部的对象呢？直觉告诉我们不能，因为`SomeBase`本身并没有提供 `begin()/end()` 接口。但如果使用 CRTP，实际上可以将其转换为派生类类型：

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

有关这种方法的具体实例，请参阅第 9 章。

#### Mixin Inheritance

在 C++ 中，可以定义一个类来继承自己的模板参数，例如

```c++
template <typename T> struct Mixin : T
{
    ...
}
```

这种方法被称为 mixin 继承，允许对类型进行分层组合。例如，你可以允许 `Foo<Bar<Baz>> x;` 声明一个实现了所有三个类的特性的类型的变量，而不必实际构造一个全新的 FooBarBaz 类型。

有关这种方法的具体示例，请参见第 9 章。

#### Properties

属性只不过是一个（通常是私有的）字段以及一个 getter 和一个 setter 的组合。在标准 C++ 中，属性的外观如下：

```c++
class Person
{
    int age;
public:
    int get_age() const { return age; }
    void set_age(int value) { age = value; }
};
```

很多语言（如 C#、Kotlin）都将属性的概念内化到编程语言中。虽然 C++ 没有这样做（而且将来也不太可能这样做），但在大多数编译器（MSVC、Clang、Intel）中都可以使用名为 property 的非标准声明规范：

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

使用方法如下：

```c++
Person person;
p.age = 20; // calls p.set_age(20)
```

### The SOLID Design Principles

SOLID 是一个缩写，代表以下设计原则（及其缩写）：

- 单一责任原则 (SRP)
- 开放-封闭原则 (OCP)
- 利斯科夫替代原则（LSP）
- 接口隔离原则（ISP）
- 依赖倒置原则（DIP）

这些原则是罗伯特-马丁（Robert C. Martin）在 2000 年代初提出的--事实上，它们只是罗伯特在其著作和博客中阐述的数十项原则中的五项。这五个特别的主题贯穿了关于模式和软件设计的讨论，因此在我们深入探讨设计模式之前（我知道你们都迫不及待了），我们将简要回顾一下 SOLID 原则的内容。

#### Single Responsibility Principle

假设你决定写一本日记，记录你最私密的想法。这本 有一个标题和若干条目。您可以将其模拟如下：

```c++
struct Journal
{
    string title;
    vector<string> entries;

    explicit Journal(const string& title) : title(title) {}
};
```

现在，您可以添加向日记账添加条目的功能，并以该条目在日记账中的序号为前缀。这很简单：

```c++
void Journal::add(const string& entry)
{
    static int count = 1;
    entries.push_back(boost::lexical_cast<string>(count++) + ": " + entry);
}
```

该日志现在可以作为

```c++
Journal j("Dear Diary");
j.add("I cried today");
j.add("I ate a bug");
```

将此函数作为日志类的一部分是合理的，因为添加日志条目是日志实际需要做的事情。日志的职责就是保存条目，因此与此相关的任何事情都可以做。

现在假设你决定将日志保存到文件中，使其持久化。在日志类中添加以下代码

```c++
void Journal::save(const string& filename)
{
    ofstream ofs(filename);
    for (auto& s : entries)
        ofs << s << endl;
}
```

这种方法有问题。日志的职责是保存日志条目，而不是将其写入磁盘。如果将磁盘写入功能添加到日志和类似类中，持久性方法的任何改变（比如，决定写入云而不是磁盘）都需要对每个受影响的类进行大量微小的修改。

我想在这里停顿一下，说明一点：一个架构如果导致你不得不对丢失的类进行大量微小的修改，无论这些类是否相关（如层次结构），这都是典型的代码问题————表明有些东西不太对劲。现在，这确实取决于具体情况：如果你要重命名一个在上百个地方使用的符号，我认为这通常是没问题的，因为 ReSharper、CLion 或任何你使用的集成开发环境都会让你执行重构，并让更改传播到所有地方。但当你需要完全重构一个界面时......那将是一个非常痛苦的过程！

因此，我认为持久性是一个单独的问题，最好用一个单独的类来表达：

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

这正是 "单一责任 "的含义：每个类只有一个责任，因此只有一个修改的理由。只有当需要对条目存储做更多事情时，才需要更改日志--例如，你可能希望每个条目都以时间戳作为前缀，因此你需要更改 add() 函数来实现这一点。另一方面，如果您想改变持久化机制，则需要在 PersistenceManager 中进行修改。

违反 SRP 的反模式的一个极端例子是 "上帝对象"。上帝对象是一个巨大的类，它试图处理尽可能多的关注点，成为一个很难处理的单体怪物。

幸运的是，"上帝对象 "很容易识别，而且由于有了源代码控制系统（只需计算成员函数的数量），可以快速识别并适当惩罚负有责任的开发人员。

#### Open-Closed Principle

假设数据库中有一系列产品（完全是假设的）。每种产品都有颜色和尺寸，其定义如下

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

现在，我们想为一组给定的产品提供某些过滤功能。我们制作一个类似于下面的过滤器：

```c++
struct ProductFilter
{
    typedef vector<Product*> Items;
};
```

现在，为了支持按颜色筛选产品，我们定义了一个成员函数来实现这一功能：

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

我们目前的方法是按颜色过滤项目，这一切都很好。我们的代码投入生产，但不幸的是，过了一段时间后，老板来了，要求我们也实现按尺寸过滤。于是我们跳回 ProductFilter.cpp，添加以下代码并重新编译：

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

这感觉就像是完全重复了，不是吗？为什么我们不直接写一个接受谓词（某个函数）的通用方法呢？其中一个原因可能是，不同形式的过滤可以用不同的方法完成：例如，某些记录类型可能有索引，需要用特定的方法进行搜索；某些数据类型适合在 GPU 上搜索，而另一些则不适合。

我们的代码投入生产，但老板再次回来告诉我们 
告诉我们现在需要按颜色和大小进行搜索。除了再添加一个函数，我们还能做什么呢？

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

从前面的情况来看，我们想要的是执行 "开放-封闭原则"（Open-Closed Principle），即一个类型对扩展是开放的，但对修改是封闭的。换句话说，我们想要的是可扩展的过滤（也许是在不同的编译单元中），而无需对其进行修改（以及重新编译已经运行并可能已发送给客户的东西）。

我们如何才能做到这一点呢？首先，我们从概念上将（SRP！）过滤过程分为两部分：过滤器（一个接收所有项目并只返回部分项目的过程）和规范（应用于数据元素的谓词定义）。

我们可以对规范接口做一个非常简单的定义：

```c++
template <typename T> struct Specification
{
    virtual bool is_satisfied(T* item) = 0;
};
```

在前面的例子中，T 类型就是我们选择的任何类型：它当然可以是一个产品，但也可以是其他东西。这使得整个方法可以重复使用。

接下来，我们需要一种基于 `Specification<T>` 的筛选方法：这可以通过定义 `Filter<T>`来实现，你猜对了：

```c++
template <typename T> struct Filter
{
    virtual vector<T*> filter(
        vector<T*> items,
        Specification<T>& spec) = 0;
};
```

同样，我们所做的只是为一个名为 filter 的函数指定签名，该函数接收所有条目和一个规范，并返回所有符合规范的项目。这里有一个假设，即条目是以向量<T*>的形式存储的，但实际上，你可以将一对迭代器或一些专门为遍历集合而设计的自定义接口传递给 filter()。遗憾的是，C++ 语言未能将枚举或集合的概念标准化，而其他编程语言（如 .NET 的 IEnumerable）中却存在这种概念。

综上所述，改进过滤器的实现其实很简单：

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

同样，你可以把传入的 `Specification<T>` 视为强类型的 std::function 等价物，它只受限于一定数量的可能过滤器规格。

现在，最简单的部分来了。要制作一个滤色器，你需要制作一个 ColorSpecification：

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

有了这个规范，再给定一个产品列表，我们现在就可以对它们进行如下筛选：

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

前者可以得到 "苹果 "和 "树"，因为它们都是绿色的。现在，我们唯一没有实现的就是搜索大小和颜色（或者，实际上，解释一下如何搜索大小或颜色，或者混合不同的条件）。答案是，您只需制定一个复合规范。例如，对于逻辑 AND，你可以这样做：

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

现在，您可以在更简单的 "规范 "基础上自由创建复合条件。重复使用我们之前制作的绿色规范，现在查找绿色大图就变得非常简单：

```c++
SizeSpecification large(Size::Large);
ColorSpecification green(Color::Green);
AndSpecification green_and_large{ large, green };

auto big_green_things = bf.filter(all, green_and_big);
for (auto& x : big_green_things)
    cout << x->name << " is large and green" << endl;

// Tree is large and green
```

这是一段很长的代码！但请记住，由于 C++ 的强大功能，您只需为两个 `Specification<T>` 对象引入一个运算符 &&，从而使通过两个（或更多！）条件进行筛选的过程变得极其简单：

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

如果现在避免为尺寸/颜色规格设置额外的变量，那么复合规格就可以缩减为一行：

```c++
auto green_and_big = ColorSpecification(Color::Green) && SizeSpecification(Size::Large);
```

让我们回顾一下什么是 OCP 原则，以及前面的示例是如何执行该原则的。从根本上说，OCP 原则就是你不需要回到你已经编写和测试过的代码中去修改它。这正是这里发生的事情！我们创建了 `Specification<T>` 和 `Filter<T>`，从那以后，我们只需实现其中任何一个接口（无需修改接口本身）即可实现新的过滤机制。这就是所谓的 "开放供扩展，封闭供修改"。

#### Liskov Substitution Principle

以芭芭拉-利斯科夫（Barbara Liskov）命名的利斯科夫替代原则（Liskov Substitution Principle）规定，如果一个接口接受一个父类对象，那么它同样也应该接受一个子类对象，而不会出现任何问题。让我们来看看 LSP 被破坏的情况。

这里有一个矩形，它有宽和高，还有一堆计算面积的 getter 和 setter：

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

现在，假设我们制作了一种特殊的矩形，叫做正方形。这个对象可以重载设置器来设置宽度和高度：

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

这种方法很邪恶。你还看不出来，因为它看起来确实很无辜：设置器只是简单地设置了两个维度，还能出什么问题呢？如果我们采用前面的方法，就可以很容易地构造出一个取矩形的函数，而这个函数在取正方形时就会爆炸：

```c++
void process(Rectangle& r)
{
    int w = r.get_width();
    r.set_height(10);

    cout << "expected area = " << (w * 10) << ", got " << r.area() << endl;
}
```

前面的函数将面积 = 宽度 × 高度作为不变式。它获取宽度，设置高度，并理所当然地期望乘积等于计算出的面积。但是，在调用前述函数时，如果使用正方形，就会出现不匹配：

```c++
Square s{5};
process(s); // expected area 50, got 25
```

从这个示例（我承认有点矫揉造作）中可以看出，process() 完全无法接受派生类型 Square 而不是基本类型 Rectangle，从而破坏了 LSP。如果你给它输入一个矩形，一切都会好起来，所以可能要过一段时间问题才会在你的测试中出现（或者在生产中--希望不会！）。

解决方案是什么？解决办法有很多。就我个人而言，我认为正方形类型根本就不应该存在：相反，我们可以创建一个同时创建矩形和正方形的工厂（参见第 3 章）：

```c++
struct RectangleFactory
{
    static Rectangle create_rectangle(int w, int h);
    static Rectangle create_square(int size);
};
```

您可能还需要一种方法来检测矩形实际上是正方形：

```c++
bool Rectangle::is_square() const
{
    return width = height;
}
```

在这种情况下，最有效的办法是在 Square 的 `set_width()/set_height()` 中抛出异常，说明这些操作不支持，应该使用 `set_size()` 代替。然而，这违反了最小意外原则，因为你会期望调用 `set_width()` 会产生有意义的变化......我说的对吗？

#### Interface Segregation Principle

好吧，这里还有一个不伦不类的例子，但也适合用来说明这个问题。假设你决定定义一台多功能打印机：一台可以打印、扫描和传真文件的设备。那么你可以这样定义它：

```c++
struct MyFavouritePrinter /* : IMachine */
{
    void print(vector<Document*> docs) override;
    void fax(vector<Document*> docs) override;
    void scan(vector<Document*> docs) override;
};
```

这样就可以了。现在，假设你决定定义一个接口，而这个接口需要由所有计划制造多功能打印机的人实现。因此，你可以在自己喜欢的集成开发环境中使用提取接口功能，得到类似下面的结果：

```c++
struct IMachine {
    virtual void print(vector<Document*> docs) = 0;
    virtual void fax(vector<Document*> docs) = 0;
    virtual void scan(vector<Document*> docs) = 0;
}
```

这是一个问题。之所以说它是个问题，是因为这个接口的某些实现者可能不需要扫描或传真，只需要打印。然而，你却强迫他们实现这些额外的功能：当然，这些功能都可以不使用，但为什么要这么麻烦呢？

因此，互联网服务提供商建议将接口拆分开来，这样实施者就可以根据自己的需要进行选择。由于打印和扫描是不同的操作（例如，扫描仪不能打印），因此我们为它们定义了不同的接口：

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

然后，打印机或扫描仪就可以实现所需的功能：

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

现在，如果我们真的需要一个 IMachine 接口，我们可以将其定义为上述接口的组合：

```c++
struct IMachine: IPrinter, IScanner /* IFax and so on */
{
};
```

当你在具体的多功能设备中实现该接口时，就可以使用这个接口。例如，您可以使用简单的委托来确保机器重复使用特定 IPrinter 和 IScanner 提供的功能：

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

因此，概括地说，这里的想法是将复杂接口的各个部分分离到不同的接口中，以避免迫使实现者实现他们并不真正需要的功能。任何时候，当你为某个复杂的应用程序编写插件时，如果给你一个包含 20 个令人困惑的函数的接口，让你去实现这些函数，而且这些函数都是无操作和返回 `nullptr` 的，那么 API 的作者很可能已经违反了 ISP。

#### Dependency Inversion Principle

DIP 的原始定义如下：

A. 高层模块不应依赖于低层模块。两者都应依赖于抽象。

这句话的基本意思是，如果您对日志记录感兴趣，您的报告组件不应依赖于具体的 ConsoleLogger，但可以依赖于 ILogger 接口。在这种情况下，我们将报告组件视为高级模块（更接近业务领域），而日志记录作为一个基本问题（有点像文件 I/O 或线程，但不完全是），则被视为低级模块。

B. 抽象不应依赖于细节。细节应依赖抽象。

这再次重申了对接口或基类的依赖优于对具体类型的依赖。希望这句话的真理是显而易见的，因为这种方法支持更好的可配置性和可测试性--只要你使用一个好的框架来为你处理这些依赖关系。

那么，现在的主要问题是：如何真正实现上述所有功能？这肯定会增加很多工作，因为现在您需要明确说明，例如，Reporting 依赖于 ILogger。表达方式可能如下：

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

现在的问题是，要初始化前一个类，需要显式调用 Reporting{ConsoleLogger{}} 或类似函数。如果 Reporting 依赖于五个不同的接口呢？如果 ConsoleLogger 本身也有依赖关系呢？您可以通过编写大量代码来处理这些问题，但还有一种更好的方法。

现代的、新潮的、时髦的方法是使用 "依赖注入"（Dependency Injection）：这主要是指使用 Boost.DI 这样的库来自动满足特定组件的依赖要求。

让我们以一辆汽车为例，这辆汽车有一个发动机，但还需要写入日志。目前，我们可以说汽车依赖于这两样东西。首先，我们可以将引擎定义为：

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

现在，我们要决定是否要提取 IEngine 界面并将其提供给汽车。也许要，也许不要，这通常是一个设计决定。如果你设想有一个引擎层次结构，或者你预计需要一个 NullEngine（见第 19 章）用于测试目的，那么是的，你确实需要抽象出接口。

无论如何，我们还需要日志记录，由于日志记录有多种方式（控制台、电子邮件、短信、飞鸽传书......），我们可能需要一个 ILogger 接口：

```c++
struct ILogger
{
    virtual ~ILogger() {}
    virtual void Log(const string& s) = 0;
};
```

以及某种具体实施：

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

现在，我们要定义的汽车既依赖于引擎，也依赖于日志组件。我们需要这两个组件，但如何存储它们完全取决于我们：我们可以使用指针、引用、`unique_ptr/shared_ptr`或其他方式。我们将把这两个依赖组件定义为构造函数参数：

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

现在，你可能希望在初始化 Car 时看到 `make_unique/make_shared` 调用。但我们不会这么做。相反，我们将使用 Boost.DI。 首先，我们将定义一个绑定，将 ILogger 绑定到 ConsoleLogger；这意味着，基本上，"只要有人要求使用 ILogger，我们就给他一个 ConsoleLogger"：

```c++
auto injector = di::make_injector(
    di::bind<ILogger>().to<ConsoleLogger>()
);
```

现在我们已经配置好了喷射器，可以用它来创建一辆汽车：

```c++
auto car = injector.create<shared_ptr<Car>>();
```

前面创建了一个 `shared_ptr<Car>` 指向一个*完全初始化*的 Car 对象，这正是我们想要的。这种方法的最大优点是，要更改使用的日志记录器类型，我们只需在一个地方（绑定调用）进行更改即可，现在出现 ILogger 的每个地方都可以使用我们提供的其他日志记录组件。这种方法还有助于我们进行单元测试，并允许我们使用存根（或空对象模式）来代替模拟。

#### Time for Patterns!

了解了 SOLID 设计原则之后，我们就可以来看看设计模式本身了。系好安全带，这将是一段漫长的旅程（但希望不会无聊）！
