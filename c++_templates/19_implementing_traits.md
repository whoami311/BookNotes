# 特征的实现

## 一个实例：累加一个序列

### 固定特征

特征模板：持有其参数类型的特征（一般来说，可能有多个特征和多个参数）。

### 值特征

特征表示与给定主类型相关的额外类型信息。这些额外的信息并不局限于类型。常量和其他类型的值也可以与类型相关联。

特征不仅仅是额外的类型。特征可以是一种机制。特征概念的关键部分在于：特征为泛型计算提供了一条配置具体元素（主要是类型）的途径。

## 特征、policy 及 policy 类

### 特征和 policy 的区别

policy 只是特征的一个特例或者说特征只用于实现 policy。

*New Shorter Oxford English Dictionary* 中对特征和 policy 的定义如下。

- 特征：用来刻画一个事物的与众不同的特性。
- policy：为了某种有益或有利的目的而采用的一系列动作。

基于上述定义，我们倾向于将 policy 类这个概念的使用表示为对某种操作的类的编码，这些操作在很大程度上同任何其他的模板参数（与之组合）都是正交的。这与 Andrei Alexandrescu 在他的 *Modern C++ Design* 中的声明是一致的：

    policy 与特征有很多共同点，不同的是，特征更注重类型，而 policy 更注重行为。

引入特征技术的 Nathan Myers 提出了下面这个更开放的定义：

    特征类：是一种用来代替模板参数的类。作为一个类，它可以是有用的类型，也可以是常量；作为一个模板，它提供了一条实现“额外层次间接性”的途径，而正是这种“额外层次间接性”解决了大量的软件问题。

一般来说，我们倾向于使用下面的（并不是非常准确的）定义。

- 特征表示模板参数的一些额外的自然属性。
- policy 表示泛型函数和泛型类的一些可配置行为（通常具有被经常使用的默认值）。

为了进一步阐述这两个概念之间可能存在的区别，我们列出了以下关于特征的观点。

- 特征可以是固定特征（fixed trait，即不能通过模板参数进行传递的特征）。
- 特征参数通常有非常自然的默认值（它很少被改写，或者根本不能被改写）。
- 特征参数往往与一个或多个主参数密切相关。
- 特征大多是对类型和常量进行组合，而不是成员函数。
- 特征通常都是用特征模板实现的。

对于 policy 类，我们发现了以下事实。

- 如果 policy 类不作为模板参数传递，那么 policy 类几乎不起作用。
- policy 参数并不需要有默认值，并且通常是显式指定的（尽管许多泛型组件配置了常用的默认 policy）。
- policy 参数主要与一个模板的其它参数正交。
- policy 一般都包含成员函数。
- policy 既可以用普通类实现，也可以用类模板实现。

显然，这两个概念之间有一条模糊的界线。

### 成员模板和模板的模板参数

通过模板的模板参数访问 policy 类的主要优点是：使 policy 类更容易携带一些状态信息（即静态数据成员），其类型取决于模板参数（静态数据成员需嵌入成员类模板中）。

然而，使用模板的模板参数的方法的一个缺点是，policy 类现在必须被写成模板，模板参数的确切个数由我们的接口定义。这会使特征本身的表达比简单的非模板类更冗长，更不自然。

### 组合多个 policy 和（或）特征

特征和 policy 并不能完全代替多个模板参数。然而，特征和 policy 确实把模板参数的个数减少到可以控制的范围内。那么，一个有趣的问题是：如何对这么多的参数进行排序？

一个简单的策略是：根据已选择并可能用于递增的参数的默认值对各个参数进行排序。通常，这意味着特征参数将位于 policy 参数的后面，因为后者在客户端代码中更经常被重写。

## 类型函数

可以对依赖于某些类型的行为进行定义。在传统意义上，在 C 和 C++ 中，我们可以定义更准确的称为值函数（value function）的函数：它们将一些值作为参数，并返回另一个值。通过模板，我们还可以定义类型函数（type function）：将某些类型作为实参，并生成一个类型或常量的函数。

### 元素类型

```c++
template <typename C>
struct ElementT {
    using Type = typename C::value_type;
};
```

```c++
template <typename T>
using ElementType = typename ElementT<T>::Type;
```

### 转换特征

除了提供对基本参数特定方面的访问之外，特征还可以对类型执行转换，例如删除或添加引用、`const` 和 `volatile` 限定符等。

1. 删除引用

```c++
template <typename T>
struct RemoveReferenceT {
    using Type = T;
};

template <typename T>
struct RemoveReferenceT<T&> {
    using Type = T;
};

template <typename T>
struct RemoveReferenceT<T&&> {
    using Type = T;
};
```

```c++
template <typename T>
using RemoveReference = typename RemoveReference<T>::Type;
```

2. 添加引用

```c++
template <typename T>
struct AddLValueReferenceT {
    using Type = T&;
};

template <typename T>
using AddLValueReference = typename AddLValueReferenceT<T>::Type;

template <typename T>
struct AddRValueReferenceT<T&> {
    using Type = T&&;
};

template <typename T>
using AddRValueReference = typename AddRValueReferenceT<T>::Type;
```

```c++
template <typename T>
using AddLValueReferenceT = T&;

template <typename T>
using AddRValueReferenceT = T&&;
```

```c++
template <>
struct AddLValueReferenceT<void> {
    using Type = void;
};

template <>
struct AddLValueReferenceT<void const> {
    using Type = void const;
};

template <>
struct AddLValueReferenceT<void volatile> {
    using Type = void volatile;
};

template <>
struct AddLValueReferenceT<void const volatile> {
    using Type = void const volatile;
};

// same as AddRValueReferenceT
```

3. 删除限定符

```c++
// traits/removeconst.hpp
template <typename T>
struct RemoveConstT {
    using Type = T;
};

template <typename T>
struct RemoveConstT<T const> {
    using Type = T;
};

template <typename T>
using RemoveConst = typename RemoveConstT<T>::Type;
```

```c++
#include "removeconst.hpp"
#include "removevolatile.hpp"

template <typename T>
struct RemoveCVT : RemoveConstT<typename RemoveVolatileT<T>::Type> {};

template <typename T>
using RemoveCV = typename RemoveCVT<T>::Type;
```

```c++
template <typename T>
using RemoveCV = RemoveConst<RemoveVolatile<T>>;
```

4. 退化

```c++
template <typename T>
struct DecayT : RemoveCVT<T> {};
```

```c++
template <typename T>
struct DecayT<T[]> {
    using Type = T*;
};

template <typename T, std::size_t N>
struct DecayT<T[N]> {
    using Type = T*;
};
```

```c++
template <typename R, typename... Args>
struct DecayT<R(Args...)> {
    using Type = R (*)(Args...);
};

template <typename R, typename... Args>
struct DecayT<R(Args..., ...)> {
    using Type = R (*)(Args..., ...);
};
```

### 谓词特征

一种特殊形式的类型特征——谓词特征（产生一个布尔值的类型函数）。

1. ```IsSameT```

```c++
template <typename T1, typename T2>
struct IsSameT {
    static constexpr bool value = false;
};

template <typename T>
struct IsSameT<T, T> {
    static constexpr bool value = true;
};
```

2. `true_type` 与 `false_type`

```c++
// traits/boolconstant.hpp
template <bool val>
struct BootConstant {
    using Type = BoolConstant<val>;
    static constexpr bool value = val;
};
using TrueType = BoolConstant<true>;
using FalseType = BoolConstant<false>;
```

```c++
#include "boolconstant.hpp"

template <typename T1, typename T2>
struct IsSameT : FalseType
{};

template <typename T>
struct IsSameT<T, T> : TrueType
{};
```

```c++
template <typename T>
using IsSame = typename IsSameT<T>::Type;
```

### 结果类型特征

C++ 标准提供了 `std::declval<>`，其在 `utility` 中的定义如下：

```c++
namespace std {
    template <typename T>
    add_rvalue_reference_t<T> declval() noexcept;
};
```

函数 `declval<T>()` 生成一个 `T` 类型的值，而不需要默认构造函数（或任何其他操作）。

上述函数模板是有意未定义的，因为它仅用于 `decltype`、`sizeof` 或其他不需要定义的上下文中。它还有两个有趣的特性。

- 对于可引用类型，返回类型始终是对该类型的右值引用，这使得 `declval` 甚至可以处理通常无法从函数返回的类型，诸如抽象类类型（具有纯虚函数的类）或数组类型。当用作表达式时，从 `T` 到 `T&&` 的转换，对 `declval<T>()` 的行为也没有实际的影响：两者都是右值（如果 `T` 是一个对象类型），而因为存在引用折叠规则，所以左值的引用类型不变。
- `noexcept` 异常规范说明了 `declval` 本身不会导致表达式被认为抛出异常。当将 `declval` 用在 `noexcept` 运算符的上下文中时，它变得很有用。

结果类型特征提供了一种确定特定操作的精确返回类型的方法。在描述函数模板的结果类型时，该方法通常是很有用的。

## 基于 SFINAE 的特征

SFINAE 原则将模板实参演绎过程中形成无效类型和表达式时出现的潜在错误（这将导致程序格式错误）转换为简单的演绎失败，从而允许重载解析选择不同的候选者。虽然 SFINAE 最初的目的是避免函数模板重载的虚假错误，但它还支持显著的编译期技术，可以确定特定类型或表达式是否有效。这允许我们编写一些特征，例如，确定一个类型是否有一个特定的成员、支持一个特定的操作或者是一个类。

基于 SFINAE 的特征的两种主要实现方法是 SFINAE 函数重载和偏特化。

### SFINAE 函数重载

第 1 种实现基于 SFINAE 的特征的方法是使用 SFINAE 函数重载——确定一个类型是否默认可构造，以便可以创建没有任何初始化值的对象。

```c++
#include "issame.hpp

template <typename T>
struct IsDefaultConstructibleT {
    private:
        // test() 尝试替换作为 U 传递的 T 的默认构造函数调用
        template <typename U, typename = decltype(U())>
        static char test(void*);
        // test() 回退
        template <typename>
        static long test(...);
    public:
        static constexpr bool value = IsSameT<decltype(test<T>(nullptr)), char>::value;
};
```

1. 基于 SFINAE 的特征的替代实现策略

```c++
enum { value = sizeof(test<...>(0)) == 1 };
```

2. 使基于 SFINAE 的特征成为谓词特征

```c++
#include <type_traits>

template <typename T>
struct IsDefaultConstructibleHelper {
    private:
        // test() 尝试替换作为 U 传递的 T 的默认构造函数调用
        template <typename U, typename = decltype(U())>
        static std::true_type test(void*);
        // test() 回退
        template <typename>
        static std::false_type test(...);
    public:
        using Type = decltype(test<T>(nullptr));
};
template <typename T>
struct IsDefaultConstructibleT : IsDefaultConstructibleHelper<T>::Type {};
```

### SFINAE 偏特化

第 2 种实现基于 SFINAE 的特征的方法是使用偏特化。同样，可以用一个实例来确定类型 `T` 是否为默认可构造的：

```c++
#include "issame.hpp"
#include <type_traits>  // 定义了 true_type 和 false_type

// helper 要忽略任意数量的模板参数
template <typename...>
using VoidT = void;

// 基本模板
template <typename, typename = VoidT<>>
struct IsDefaultConstructibleT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T>
struct IsDefaultConstructibleT<T, VoidT<decltype(T())>> : std::true_type {};
```

### 为 SFINAE 使用泛型 lambda 表达式

在 C++17 中通过指定在泛型 lambda 表达式中检查的条件，来使样例代码最小化。

```c++
#include <utility>

// helper: 对于 F f 和 Args... args，检查 f(args...) 的有效性

template <typename F, typename... Args, typename = decltype(std::declval<F>()(std::declval<Args&&>()...))>
std::true_type isValidImpl(void*);

// 如果 helper 被 SFINAE 取消，则回退
template <typename F, typename... Args>
std::false_type isValidImpl(...);

// 定义一个 lambda，它接收 lambda f，并返回用 args 调用 f 是否有效
inline constexpr auto isValid = [] (auto f) {
    return [] (auto&&... args) {
        return decltype(isValidImpl<decltype(f), decltype(args)&&...>(nullptr)) {};
    };
};

// helper 模板将类型表示为一个值
template <typename T>
struct TypeT {
    using Type = T;
};

// helper 将类型包装为值
template <typename T>
constexpr auto type = TypeT<T>{};

// helper 在未计算的上下文中展开包装类型
template <typename T>
T valueT(TypeT<T>); // 不需要定义
```

```c++
isDefaultConstructible(type<int>)   // true （int 是默认可构造的）
isDefaultConstructible(type<int&>)  // false （引用不是默认可构造的）
```

`isDefaultConstructible` 特征与以前的特征的实现稍有不同，因为它需要函数风格的调用，而不是指定模板实参。这可以说是一种更易于阅读的表示法，但也可以使用先前的风格。

```c++
template <typename T>
using IsDefaultConstructibleT = decltype(isDefaultConstructible(std::declval<T>()));
```

但是，由于这是一个传统的模板声明，因此，它只能出现在命名空间作用域中，而可以想象，`isDefaultConstructible` 的定义是在块的作用域中引入的。

到目前为止，这项技术似乎并没有引起人们的注意，因为实现中涉及的表达式和使用风格都比以前的技术要更为复杂。然而，一旦 `isValid` 就位并被理解，许多特征就可以通过一个声明来实现。例如：

```c++
constexpr auto hasFirst = isValid([] (auto x) -> decltype((void)valueT(x).first) {});
```

### SFINAE 友好的特征

一般来说，一个类型特征应该能够解决一个特定的查询问题，而不会导致程序的格式不合规。基于 SFINAE 的特征通过在 SFINAE 环境中小心地捕捉潜在的问题来解决这个问题，将那些可能出现的错误转化为负面结果。

```c++
// traits/hasplus.hpp
#include <utility>      // 对于 declval
#include <type_traits>  // 对于 true_type、false_type 和 void_t

// 基本模板
template <typename, typename, typename = std::void_t<>>
struct HasPlusT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T1, typename T2>
struct HasPlusT<T1, T2, std::void_t<decltype(std::declval<T1>() + std::declval<T2>())>> : std::true_type {};
```

```c++
#include "hasplus.hpp"

template <typename T1, typename T2, bool = HasPlusT<T1, T2>::value>
struct PlusResultT { // 基本模板：当 HasPlusT 生成 true 时使用
    using Type = decltype(std::declval<T1>() + std::declval<T2>());
};

template <typename T1, typename T2>
struct PlusResultT<T1, T2, false> { // 否则使用偏特化
};
```

作为一个常见的设计原则，如果给定合理的模板实参作为输入，一个特征模板在实例化时不应该失败。通常的方法是执行两次相应的检查：

- 一次检查操作是否有效
- 一次计算结果

```c++
template <typename C, bool = HasMemberT_value_type<C>::value>
struct ElementT {
    using Type = typename C::value_type;
};

template <typename C>
struct ElementT<C, false> {};
```

## `IsConvertibleT`

```c++
#include <type_traits>  // 对于 true_type 和 false_type
#include <utility>      // 对于 declval

template <typename FROM, typename TO>
struct IsConvertibleHelper {
    private:
        // test() 尝试调用作为 F 传递的 FROM 的 helper 对象 aux(TO)
        static void aux(TO);
        template <typename F, typename T,
                  typename = decltype(aux(std::decltype<F>()))>
        static std::true_type test(void*);

        // test() 回退
        template <typename, typename>
        static std::false_type test(...);
    
    public:
        using Type = decltype(test<FROM>(nullptr));
};

template <typename FROM, typename TO>
struct IsConvertibleT : IsConvertibleHelper<FROM, TO>::Type {};

template <typename FROM, typename TO>
using IsConvertible = typename IsConvertibleT<FROM, TO>::Type;

template <typename FROM, typename TO>
constexpr bool isConvertible = IsConvertibleT<FROM, TO>::value;
```

```c++
template <typename FROM, typename TO, bool = IsVoidT<TO>::value
                                             || IsArrayT<TO>::value
                                             || IsFunctionT<TO>::value>
struct IsConvertibleHelper {
    using Type = std::integral_constant<bool,
                                        IsVoidT<TO>::value
                                        && IsVoidT<FROM>::value>;
};

template <typename FROM, typename TO>
struct IsConvertibleHelper<FROM, TO, false> {
    ... // 这是 IsConvertibleHelper 以前的实现代码
};
```

## 检测成员

针对基于 SFINAE 的特征的另一个尝试涉及创建一个特征（或者更确切地说，一组特征），它可以确定一个给定类型 `T` 是否有一个名称为 `X` 的成员（一个类型或一个非类型成员）。

### 检测成员类型

```c++
#include <type_traits>  // true_type 和 false_type 的定义

// 忽略任意个数的模板参数的 helper
template <typename...>
using VoidT = void;

// 基本模板
template <typename, typename = VoidT<>>
struct HasSizeTypeT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T>
struct HasSizeTypeT<T, VoidT<typename T::size_type>> : std::true_type {};
```

1. 处理引用类型

```c++
template <typename T>
struct HasSizeTypeT<T, VoidT<RemoveReference<T>::size_type>> : std::true_type {};
```

2. 插入式类名

我们检查成员类型的特征技术也会为插入式类名生成 `true` 值。

```c++
struct size_type {};

struct Sizeable : size_type {};

static_assert(HasSizeTypeT<Sizeable>::value,
              "Compiler bug: Injected class name missing");
```

后一个静态断言成功，因为 `size_type` 将自己的名称作为成员类型引入，并且该名称是继承的。
