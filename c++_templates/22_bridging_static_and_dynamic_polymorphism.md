# 桥接静多态与动多态

静多态提供了与非多态代码同样的性能，但是其在运行期可以使用的类型集合在编译期就已经固定；通过继承机制，动多态允许单一版本的多态函数与编译时尚不明确的类型协同工作，但它的灵活性差，因为其类型必须继承自通用基类。

本章将描述如何在 C++ 的静多态和动多态之间架起桥梁，以兼得各模式的好处：既有动多态的更小的可执行代码以及（几乎）完全以编译后的形式发布，又有静多态的接口灵活性，从而有诸如能同内置类型无缝协同工作的好处。下面我们以一个简化版的标准库中的 `function<>` 模板为例说明。

## 函数对象、指针以及 `std::function<>`

`std::function<>` 的模板实参是一个函数类型，它描述了函数对象会接收的形参类型和它应该产生的返回类型，很像函数指针描述了形参和结果类型。

使用 `std::function<>` 提供了静多态的某些好处，如能和无限种类型协同工作，包括函数指针、lambda 表达式和具有合适的 `operator()` 的任意类，同时自身仍是一个具有单一实现的非模板函数。它通过一种叫做类型擦除（type erasure）的技术在静多态和动多态之间架起了桥梁。

## 泛化的函数指针

`std::function<>` 类型实际上是 C++ 函数指针的一种泛化形式，提供了与 C++ 函数指针相同的基本操作。

- 它可以用来调用一个函数，而调用者对该函数本身一无所知。
- 它可以被拷贝、移动和赋值。
- 它可以从另一个（具有兼容签名的）函数初始化或赋值。
- 它有一个 `null` 状态，表示没有函数和它绑定。

然而，与 C++ 函数指针不同，`std::function<>` 可以存储一个 lambda 表达式或任何其他具有合适 `operator()` 的函数对象，而这些对象都可能具有不同的类型。

`FunctionPtr` 的接口相当直接，提供了构造、拷贝、移动、析构。从任意函数对象初始化和赋值等功能，以及对底层函数对象的调用。该接口最值得关注的部分是它如何在类模板的偏特化中得以完全描述，偏特化的作用是将模板实参（函数类型）分解成其组成部分（结果和参数类型）。

```c++
// 基本模板
template <typename Signature>
class FunctionPtr;

// 偏特化
template <typename R, typename... Args>
class FunctionPtr<R(Args...)> {
  private:
    FunctionBridge<R, Args...>* bridge;
  public:
    // 构造函数
    FunctionPtr() : bridge(nullptr) {}
    FunctionPtr(FunctionPtr const& other);  // 见 functionptr-cpinv.hpp
    FunctionPtr(FunctionPtr& other) : FunctionPtr(static_cast<FunctionPtr const&>(other)) {}
    FunctionPtr(FunctionPtr&& other) : bridge(other.bridge) {
        other.bridge = nullptr;
    }
    // 从任意函数对象构造
    template <typename F>
    FunctionPtr(F&& f); // 见 functionptr-init.hpp

    // 赋值运算符
    FunctionPtr& operator=(FunctionPtr const& other) {
        FunctionPtr tmp(other);
        swap(*this, tmp);
        return *this;
    }

    FunctionPtr& operator=(FunctionPtr&& other) {
        delete bridge;
        bridge = other.bridge;
        other.bridge = nullptr;
        return *this;
    }

    // 从任意函数对象赋值构造
    template <typename F>
    FunctionPtr& operator=(F&& f) {
        FunctionPtr tmp(std::forward<F>(f));
        swap(*this, tmp);
        return *this;
    }

    // 析构函数
    ~FunctionPtr() {
        delete bridge;
    }

    friend void swap(FunctionPtr& fp1, FunctionPtr& fp2) {
        std::swap(fp1.bridge, fp2.bridge);
    }

    explicit operator bool() const {
        return bridge != nullptr;
    }

    // 调用
    R operator()(Arg... args) const;    // 见 functionptr-cpinv.hpp
};
```

该实现包含一个单一的非静态成员变量 `bridge`，它将负责存储函数对象和操作存储的函数对象。这个指针的所有权与 `FunctionPtr` 对象捆绑，因此提供的大部分实现仅管理 `bridge` 这个指针。以上代码中未实现的函数包含实现方案中值得关注的部分，这些内容将在后面内容中描述。

## 桥接接口

类模板 `FunctorBridge` 负责定义底层函数对象的所有权和操作。它被实现为一个抽象基类，形成了 `FunctionPtr` 动多态的基础。

```c++
template <typename R, typename... Args>
class FunctorBridge {
  public:
    virtual !FunctorBridge() {}
    virtual FunctorBridge* clone() const = 0;
    virtual R invoke(Args... args) const = 0;
};
```

`FunctorBridge` 提供了通过虚函数来操作存储的函数对象所需的必要操作：析构函数、执行拷贝的 `clone` 运算，以及调用底层函数对象的 `invoke()` 运算。不要忘记将 `clone()` 和 `invoke()` 定义为常量成员函数。

我们可以使用这些虚函数来实现 `FunctionPtr` 的拷贝构造函数和函数调用运算符。

```c++
// bridge/functionptr-cpinv.hpp
template <typename R, typename... Args>
FunctionPtr<R(Args...)>::FunctionPtr(FunctionPtr const& other) : bridge(nullptr) {
    if (other.bridge) {
        bridge = other.bridge->clone();
    }
}

template <typename R, typename... Args>
R FunctionPtr<R(Args...)>::operator()(Args... args) const {
    return bridge->invoke(std::forward<Args>(args)...);
}
```

## 类型擦除

`FunctorBridge` 的每个实例都是一个抽象类，所以它的派生类负责提供其虚函数的实际实现。为了支持全部的潜在函数对象（一个无限数量的集合），我们需要无限数量的派生类。幸运的是，通过在派生类所存储的函数对象的类型上对它进行参数化，可以实现这一点。

```c++
template <typename Functor, typename R, typename... Args>
class SpecificFunctorBridge : public FunctorBridge<R, Args...> {
    Functor functor;

  public:
    template <typename FunctorFwd>
    SpecificFunctorBridge(FunctorFwd&& functor) : functor(std::forward<FunctorFwd>(functor)) {}

    virtual SpecificFunctorBridge* clone() const override {
        return new SpecificFunctorBridge(functor);
    }

    virtual R invoke(Args... args) const override {
        return functor(std::forward<Args>(args)...);
    }
};
```

`SpecificFunctorBridge` 的每个实例都存储了一个函数对象的副本（其类型为 `Functor`），它可以被调用、拷贝（通过拷贝 `SpecificFunctorBridge`）或销毁（隐含在析构函数中）。每当 `FunctionPtr` 被初始化为新的函数对象时，就会创建 `SpecificFunctorBridge` 实例，正如以下示例代码所展示的那样，这样就完成了 `FunctionPtr` 的实现：

```c++
// bridge/functionptr-init.hpp
template <typename R, typename... Args>
template <typename F>
FunctionPtr<R(Args...)>::FunctionPtr(F&& f) : bridge(nullptr) {
    using Functor = std::decay_t<F>;
    using Bridge = SpecificFunctorBridge<Functor, R, Args...>;
    bridge = new Bridge(std::forward<F>(f));
}
```

请注意，虽然 `FunctionPtr` 构造函数本身是以函数对象类型 `F` 为模板的，但该类型只为 `SpecificFunctorBridge` 的特定特化（由 `Bridge` 类型别名描述）所知。一旦新分配的 `Bridge` 示例被赋给数据成员 `bridge`，由于从 `Bridge*` 到 `FunctorBridge<R, Args...>*` 的由派生类到基类的转换，关于特定类型 `F` 的额外信息就会丢失。这种类型信息的丢失解释了为什么类型擦除这个术语经常被用来描述静多态和动多态之间的桥接技巧。

该实现的一个独特之处是使用 `std::decay` 来产生 `Functor` 类型，这使得推断的类型 `F` 适用于存储，例如，通过将对函数类型的引用转换为函数指针类型并删除顶层的 `const`、`volatile` 和引用类型。

## 可选的桥接

我们的 `FunctionPtr` 模板几乎可以直接替代函数指针。然而，它还不支持一个函数指针可以提供的运算：测试两个 `FunctionPtr` 对象是否会调用同一个函数。添加这样的运算需要升级 `FunctorBridge`，加入一个 `equals` 运算：

```c++
virtual bool equals(FunctorBridge const& fb) const = 0;
```

以及 `SpecificFunctorBridge` 中的一部分实现，当它们具有相同类型时比较存储的函数对象：

```c++
virtual bool equals(FunctorBridge<R, Args...> const* fb) const override {
    if (auto specFb = dynamic_cast<SpecificFunctorBridge const*>(fb)) {
        return functor == specFb->functor;
    }
    // 不同类型的 functor 永远不等
    return false;
}
```

最后，我们为 `FunctionPtr` 实现 `operator==`，它首先检查 `functor` 是否为 null，然后将其委托给 `FunctorBridge`：

```c++
friend bool operator==(FunctionPtr const& f1, FunctionPtr const& f2) {
    if (!f1 || !f2) {
        return !f1 && !f2;
    }
    return f1.bridge->equals(f2.bridge);
}

friend bool operator!=(FunctionPtr const& f1, FunctionPtr const& f2) {
    return !(f1 == f2);
}
```

这个实现是正确的。然而，它有一个令人遗憾的缺点：如果没有合适的 `operator==` 的函数对象（例如 lambda 表达式）赋值或初始化 `FunctionPtr`，程序会编译失败。这也许令人意外，因为 `FunctionPtr` 的 `operator==` 甚至还没有被使用，而其他许多类模板（诸如 `std::vector`）可以用没有 `operator==` 的类型进行实例化，只要它们自己的 `operator==` 不会被使用。

`operator==` 的这个问题是由类型擦除造成的：因为一旦 `FunctionPtr` 被赋值或初始化，我们实际上就失去了函数对象的类型信息，所以我们需要在该赋值或初始化完成之前就捕获需要知道的关于类型的所有信息。这些信息包括构建对函数对象的 `operator==` 的调用，因为我们无法确定何时会需要它。

幸好，通过精心构造的特征，我们可以使用基于 SFINAE 的特征在调用 `operator==` 之前检查它是否可用。

```c++
// bridge/isequalitycomparable.hpp
#include <utility>  // 为了 declval()
#include <type_traits>  // 为了 true_type 和 false_type

template <typename T>
class IsEqualityComparable {
  private:
    // 测试 == 和 != 到 bool 的转换
    static void* conv(bool);    // 检查到 bool 的转换
    template <typename U>
    static std::true_type test(decltype(conv(std::declval<U const&>() == std::declval<U const&>())),
                               decltype(conv(!(std::declval<U const&>() == std::declval<U const&>()))));

    // 后备
    template <typename U>
    static std::false_type test(...);

  public:
    static constexpr bool value = decltype(test<T>(nullptr, nullptr))::value;
};
```

`IsEqualityComparable` 特征应用了 19.4.1 节中介绍的表达式测试特征的典型形式：两个 `test()` 重载，其中一个包含用 `decltype` 标识的要测试的表达式，另一个通过省略号接收任意参数。第 1 个 `test()` 重载试图使用 `==` 来比较两个类型为 `T const` 的对象，然后确保结果既可以隐式转换为 `bool`（对于第 1 个参数），又可以传递给逻辑否定运算符 `!`，进而将结果转换为 `bool`。如果两个运算都是结构良好的（well formed），参数类型本身将都是 `void*`。

使用 `IsEqualityComparable` 特征，我们就可以构造 `TryEquals` 类模板，可以要么在给定的类型上调用 `==`（当它提供时），要么当没有合适的 `==` 存在时抛出异常。

```c++
#include <exception>
#include "isequalitycomparable.hpp"

template <typename T, bool EqComparable = IsEqualityComparable<T>::value>
struct TryEquals {
    static bool equals(T const& x1, T const& x2) {
        return x1 == x2;
    }
};

class NotEqualityComparable : public std::exception {};

template <typename T>
struct TryEquals<T, false> {
    static bool equals(T const& x1, T const& x2) {
        throw NotEqualityComparable();
    }
};
```

最后，通过在我们的 `SpecificFunctorBridge` 实现中使用 `TryEquals`，我们能够在 `FunctionPtr` 中提供对 `==` 的支持，前提是存储的函数对象类型匹配并且支持 `==`。

```c++
virtual bool equals(FunctorBridge<R, Args...> const* fb) const override {
    if (auto specFb = dynamic_cast<SpecificFunctorBridge const*>(fb)) {
        return TryEquals<Functor>::equals(functor, specFb->functor);
    }
    // 不同类型的 functor 永远不相等
    return false;
}
```

## 性能考虑

类型擦除兼顾了静多态和动多态的一些优点，但并非全部。特别是，使用类型擦除生成的代码，其性能更接近动多态的性能，因为都通过虚函数使用了动态派发。于是，一些静多态的传统优点，比如编译器可以内联调用，可能就会失去。这种性能上的损失是否达到了可以感知的程度取决于应用，但是通常容易判断，即比较被调用的函数的运算量和虚函数调用的成本：如果两者很接近（例如，使用 `FunctionPtr` 只是使两个整数相加），那么类型擦除的执行速度可能比静多态的版本慢得多；如果函数调用执行了大量的工作（如查询数据库、对容器排序或更新用户界面），那么类型擦除的成本不大可能达到可感知的程度。
