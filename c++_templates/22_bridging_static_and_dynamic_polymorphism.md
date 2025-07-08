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
