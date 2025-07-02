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
// traits/isvalid.hpp
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

### 检测任意的成员类型

如何参数化特征，以便能够检测“任意”成员类型名称。

遗憾的是，这目前只能通过宏来实现，因为还没有语言机制来表述“潜在”名称。目前，在不使用宏的情况下，最接近的方法是使用泛型 lambda。

```c++
#include <type_traits>  // 对于 true_type、false_type 和 void_t

#define DEFINE_HAS_TYPE(MemType)                                    \
  template <typename, typename = std::void_t<>>                     \
  struct HasTypeT_##MemType                                         \
    : std::false_type {};                                           \
  template <typename T>                                             \
  struct HasTypeT_##MemType<T, std::void_t<typename T::MemType>>    \
    : std::true_type {}     //; 有意跳过
```

### 检测 nontype 成员

```c++
#include <type_traits>  // 对于 true_type、false_type 和 void_t

#define DEFINE_HAS_MEMBER(Member)                                   \
  template <typename, typename = std::void_t<>>                     \
  struct HasMemberT_##Member                                        \
    : std::false_type {};                                           \
  template <typename T>                                             \
  struct HasMemberT_##Member<T, std::void_t<decltype(&T::Member)>>  \
    : std::true_type {}     //; 有意跳过
```

在这里，当 `&T::Member` 无效时，我们使用 SFINAE 禁用偏特化。要使该构造有效，必须满足以下条件：

- 成员必须明确地标识 `T` 的成员名称（例如，它不能是重载成员函数名，也不能是相同名称的多个继承成员的名称）
- 成员必须可以访问
- 成员必须是非类型、非枚举成员（否则前缀 `&` 将无效）
- 如果 `T::Member` 是静态数据成员，则其类型不能提供使 `&T::Member` 无效（例如，使其不可访问）的运算符 `&`。

修改偏特化以排除 `T::Member` 不是指向成员类型的指针（相当于排除静态数据成员）的情况并不困难。类似地，可以排除指向成员函数的指针，或者要求它将特征限制为数据成员或成员函数。

1. 检测成员函数

`HasMember` 特征只检查是否存在具有相应名称的单个成员。如果存在两个成员，这个特征也会失败。比如重载的成员函数。

SFINAE 原则可以防止在函数模板声明中创建无效类型和表达式的尝试，从而允许上述重载技术扩展到测试任意表达式的格式是否是符合语法的。

```c++
#include <utility>      // 对于 declval
#include <type_traits>  // 对于 true_type、false_type 和 void_t

// 基本模板
template <typename, typename = std::void_t<>>
struct HasBeginT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T>
struct HasBeginT<T, std::void_t<decltype(std::declval<T>().begin())>>
  : std::true_type {};
```

2. 检测其他表达式

我们可以将上述技术用于其他类型的表达式，甚至可以组合多个表达式。

```c++
#include <utility>      // 对于 declval
#include <type_traits>  // 对于 true_type、false_type 和 void_t

// 基本模板
template <typename, typename, typename = std::void_t<>>
struct HasLessT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T1, typename T2>
struct HasLessT<T1, T2, std::void_t<decltype(std::declval<T1>() < std::declval<T2>())>>
  : std::true_type {};
```

由于 `std::void_t` 的性质，我们可以在一个特征中组合多个约束。

```c++
#include <utility>      // 对于 declval
#include <type_traits>  // 对于 true_type、false_type 和 void_t

// 基本模板
template <typename, typename = std::void_t<>>
struct HasVariousT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T>
struct HasVariousT<T, std::void_t<decltype(std::declval<T>().begin()),
                                  typename T::difference_type,
                                  typename T::iterator>>
  : std::true_type {};
```

检测特定语法有效性的特征非常强大，它允许一个模板根据特定操作的存在或不存在自定义其行为。这些特征将再次被用作 SFINAE 友好的特征定义的一部分和基于类型属性的重载辅助。

### 使用泛型 lambda 检测成员

“为 SFINAE 使用泛型 lambda 表达式”一节中介绍的名为 `isValid` 的 lambda 表达式提供了一种更紧凑的技术来定义检查成员的特征，有助于避免使用宏来处理任意名称的成员。

```c++
#include "isvalid.hpp"
#include <iostream>
#include <string>
#include <utility>

int main() {
    using namespace std;
    cout << boolalpha;

    // 定义以检查数据成员 first
    constexpr auto hasFirst
      = isValid([] (auto x) -> decltype((void)valueT(x).first) {});
    
    cout << "hasFirst: " << hasFirst(type<pair<int, int>>) << '\n'; // true

    // 定义以检查类型成员 size_type
    constexpr auto hasSizeType
      = isValid([] (auto x) -> typename decltype(valueT(x))::size_type {});
    
    struct CX {
        using size_type = std::size_t;
    };
    cout << "hasSizeType: " << hasSizeType(type<CX>) << '\n';   // true

    if constexpr (!hasSizeType(type<int>)) {
      cout << "int has no size_type\n";
      ...
    }

    // 定义以检查 <
    constexpr auto hasLess
      = isValid([] (auto x, auto y) -> decltype(valueT(x) < valueT(y)) {});
    
    cout << hasLess(42, type<char>) << '\n';                // 输出 true
    cout << hasLess(type<string>, type<string>) << '\n';    // 输出 true
    cout << hasLess(type<string>, type<int>) << '\n';       // 输出 false
    cout << hasLess(type<string>, "hello") << '\n';         // 输出 true
}
```

为了能够使用泛型语法，将类型作为模板参数，我们可以再次定义额外的 helper。

```c++
#include "isvalid.hpp"
#include <iostream>
#include <string>
#include <utility>

constexpr auto hasFirst
  = isValid([] (auto&& x) -> decltype((void)&x.first) {});

template <typename T>
using HasFirstT = decltype(hasFirst(std::declval<T>()));

constexpr auto hasSizeType
  = isValid([] (auto&& x)
            -> typename std::decay_t<decltype(x)>::size_type {});

template <typename T>
using HasSizeTypeT = decltype(hasSizeType(std::declval<T>()));

constexpr auto hasLess
  = isValid([] (auto&& x, auto&& y) -> decltype(x < y) {});

template <typename T1, typename T2>
using HasLessT = decltype(hasLess(std::declval<T1>(), std::declval<T2>()));

int main() {
  using namespace std;

  cout << "first: " << HasFirstT<pair<int, int>>::value << '\n';    // true

  struct CX {
    using size_type = std::size_t;
  };

  cout << "size_type: " << HasSizeTypeT<CX>::value << '\n';     // true
  cout << "size_type: " << HasSizeTypeT<int>::value << '\n';    // false

  cout << HasLessT<int, char>::value << '\n';                   // true
  cout << HasLessT<string, string>::value << '\n';              // true
  cout << HasLessT<string, int>::value << '\n';                 // false
  cout << HasLessT<string, char*>::value << '\n';               // true
}
```

## 其他特征技术

## if-then-else

我们可以用一个特殊的类型模板 `IfThenElse` 来描述 if-then-else 行为，`IfThenElse` 接收一个布尔非类型模板参数来选择两个类型参数之一：

```c++
// 基本模板：默认情况下产生第 2 个实参并依赖于产生第 3 个实参的偏特化
// 假设 COND 为 false
template <bool COND, typename TrueType, typename FalseType>
struct IfThenElseT {
    using Type = TrueType;
};

// 偏特化：错误产生第 3 个实参
template <typename TrueType, typename FalseType>
struct IfThenElseT<false, TrueType, FalseType> {
    using Type = FalseType;
};

template <bool COND, typename TrueType, typename FalseType>
using IfThenElse = typename IfThenElseT<COND, TrueType, FalseType>::Type;
```

与普通 C++ 的 if-then-else 语句不同，由于这里的 if-then-else 语句在选择之前对 “then” 和 “else” 分支的模板实参进行预测，因此两个分支都可能包含不规范的代码，或者程序可能是不合规的。

```c++
// 使用成员类型生成 T
template <typename T>
struct IdentityT {
  using Type = T;
};

// 在 IfThenElse 计算后生成 unsigned
template <typename T>
struct MakeUnsignedT {
  using Type = typename std::make_unsigned<T>::type;
};

template <typename T>
struct UnsignedT {
  using Type = typename IfThenElse<std::is_integral<T>::value
                                    && !std::is_same<T, bool>::value,
                                    MakeUnsignedT<T>,
                                    IdentityT<T>
                                    >::Type;
};
```

这完全依赖一个事实，即 `IfThenElse` 构造中未选择的包装器类型从未完全实例化。

### 检测 nonthrowing 操作

```c++
#include <utility>      // 对于 declval
#include <type_traits>  // 对于 true_type、false_type 和 bool_constant<>

// 基本模板
template <typename T, typename = std::void_t<>>
struct IsNothrowMoveConstructibleT : std::false_type {};

// 偏特化（可能会被 SFINAE 取消）
template <typename T>
struct IsNothrowMoveConstructibleT<T, std::void_t<decltype(T(std::declval<T>()))>>
  : std::bool_constant<noexcept(T(std::declval<T>()))> {};
```

如果不能直接调用移动构造函数，就无法检查它是否抛出异常。也就是说，移动构造函数是公共且不被删除的仍是不够的，它还要求相应的类型不是抽象类（对抽象类的引用或指针可以正常执行）。

### 特征的便利性

类型特征的一个常见问题是相对冗长，因为每次使用类型特征都需要在尾部添加 `::Type`，在依赖上下文中添加前导 `typename` 关键字，这两个关键字都是范例。当组合多个类型特征时，如果我们正确地实现它，并确保不返回常量或引用类型，可能会强制进行一些笨拙的格式化。

通过使用别名模板和变量模板，可以方便地分别使用特征、生成类型或值。但是，请注意，在某些上下文中，这些快捷方式并不可用，我们必须使用原始的类模板。我们接下来将讨论更通用的情况。

1. 别名模板和特征

别名模板提供了一种减少冗长的方法。我们可以直接使用别名模板，而不是将类型特征表示为具有类型成员 `Type` 的类模板。

但是，对类型特征使用别名模板也有缺点，如下所示。

- 别名模板不能被特化，而且由于编写特征的许多技术都依赖于特化，因此别名模板可能需要重定向到类模板。
- 有些特征是由用户特化的，例如描述特定加法操作是否可交换的特征，当大多数使用涉及别名模板时，特化类模板可能会造成混淆。
- 使用别名模板将始终实例化类型（例如，基础类模板特化），这使得很难避免实例化对给定类型没有意义的特征。

表达最后一点的另一种方式是，别名模板不能和元函数转发同时使用。

因为使用别名模板来处理类型特征既有积极的一面，又有消极的一面，所以我们建议在本节中使用它们，正如在 C++ 标准库中所做的那样：为两个类模板提供特定的命名约定（我们选择了 `T` 后缀和 `Type` 类型成员）和别名模板〔它们的命名约定略有不同（我们删除了 `T` 后缀）〕，并根据底层类模板定义每个别名模板。这样，我们可以在别名模板提供更清晰的代码的地方使用别名模板。但为了更高级地使用别名模板，可以返回类模板。

由于历史原因，C++ 标准库有不同的约定。特征类模板产生一个类型的类型，没有特定的后缀（C++11 中引入了许多）。相应的别名模板（直接生成类型）开始在 C++14 中引入，并给出了 `_t` 后缀，因为未加后缀的名称已经被标准化。

2. 变量模板和特征

返回值的特征需要在尾部添加 `::value`（或类似的成员选择）来生成特征的结果。在这种情况下，`constexpr` 变量模板提供了一种减少这种冗长的方法。

同样，由于历史原因，C++ 标准库有不同的约定。生成结果值的特征类模板没有特定的后缀，其中许多都是在 C++11 标准中引入的。直接生成结果值的相应变量模板在 C++17 中引入了 `_v` 后缀。

## 类型分类

我们将开发一套类型特征，用于确定给定类型的各种属性。基于此，我们将能够编写特定于某些类型的代码：

```c++
if (IsClassT<T>::value) {
    ...
}
```

或者使用自 C++17 开始可用的编译期 `if` 和特征的便利特性：

```c++
if constexpr (IsClass<T>) {
    ...
}
```

或者使用偏特化：

```c++
template <typename T, bool = IsClass<T>>
class C {               // 基本模板
    ...
};

template <typename T>
class C<T, true> {      // 对于类类型的偏特化
    ...
};
```

此外，`IsPointerT<T>::value` 这样的表达式将是布尔常量，这些常量是有效的非类型模板实参。这允许我们构造更复杂、功能更强大的模板，根据其类型实参的属性特化其行为。

C++ 标准库定义了几个类似的特征来确定类型的基本类型和复合类型。

### 确定基本类型

```c++
#include <cstddef>  // 对于 nullptr_t
#include <type_traits>  // 对于 true_type、false_type 和 bool_constant<>

// 基本模板：一般来说，T 不是一个基本类型
template <typename T>
struct IsFundaT : std::false_type {};

// 专门用于基本类型的宏
#define MK_FUNDA_TYPE(T)    \
  template <> struct IsFundaT<T> : std::true_type {};

MK_FUNDA_TYPE(void)

MK_FUNDA_TYPE(bool)
MK_FUNDA_TYPE(char)
MK_FUNDA_TYPE(signed char)
MK_FUNDA_TYPE(unsigned char)
MK_FUNDA_TYPE(wchar_t)
MK_FUNDA_TYPE(char16_t)
MK_FUNDA_TYPE(char32_t)

MK_FUNDA_TYPE(signed short)
MK_FUNDA_TYPE(unsigned short)
MK_FUNDA_TYPE(signed int)
MK_FUNDA_TYPE(unsigned int)
MK_FUNDA_TYPE(signed long)
MK_FUNDA_TYPE(unsigned long)
MK_FUNDA_TYPE(signed long long)
MK_FUNDA_TYPE(unsigned long long)

MK_FUNDA_TYPE(float)
MK_FUNDA_TYPE(double)
MK_FUNDA_TYPE(long double)

MK_FUNDA_TYPE(std::nullptr_t)

#undef MK_FUNDA_TYPE
```

C++ 标准库使用的是一种更细粒度的方法，而不只是检查一个类型是否为基本类型。它首先定义基本类型的种类，其中每个类型正好匹配一个类型的种类，然后定义复合类型的种类，如 `std::is_integral` 或 `std::is_fundamental`。

### 确定复合类型

复合类型是从其他类型构造的类型。简单的复合类型包括指针类型、左值和右值引用类型、指向成员的指针类型和数组类型。它们由一个或多个基本类型构成。类类型和函数类型也是复合类型，它们的组合可以包含任意数量的基本类型（对于参数或成员）。在这个分类中，枚举类型也被视为非简单复合类型，即使它们不是由多个基本类型组成的复合类型。简单的复合类型可以使用偏特化进行分类。

1. 指针类型

```c++
template <typename T>
struct IsPointerT : std::false_type {   // 基本模板：默认没有指针
};

template <typename T>
struct IsPointerT<T*> : std::true_type {    // 对于指针的偏特化
    using BaseT = T;    // 类型指向
};
```

2. 引用类型

```c++
// traits/islvaluereference.hpp
template <typename T>
struct IsLValueReferenceT : std::false_type {   // 默认没有左值引用
};

template <typename T>
struct IsLValueReferenceT<T&> : std::true_type {    // 除非 T 是左值引用
    using BaseT = T;                                // 类型引用
};
```

```c++
// traits/isrvaluereference.hpp
template <typename T>
struct IsRValueReferenceT : std::false_type {   // 默认没有右值引用
};

template <typename T>
struct IsRValueReferenceT<T&&> : std::true_type {    // 除非 T 是右值引用
    using BaseT = T;                                // 类型引用
};
```

```c++
#include "islvaluereference.hpp"
#include "isrvaluereference.hpp"
#include "ifthenelse.hpp"

template <typename T>
class IsReferenceT
    : public IfThenElseT<IsLValueReferenceT<T>::value,
                         IsLValueReferenceT<T>,
                         IsRValueReferenceT<T>
                        >::Type {};
```

3. 数组类型

```c++
#include <cstddef>

template <typename T>
struct IsArrayT : std::false_type {         // 基本模板：不是数组
};

template <typename T, std::size_t N>
struct IsArrayT<T[N]> : std::true_type {    // 对于数组的偏特化
    using BaseT = T;
    static constexpr std::size_t size = N;
};

template <typename T>
struct IsArrayT<T[]> : std::true_type {     // 非绑定数组的偏特化
    using BaseT = T;
    static constexpr std::size_t size = 0;
};
```

4. 指向成员的指针类型

```c++
template <typename T>
struct IsPointerToMemberT : std::false_type {           // 默认没有指向成员的指针
};

template <typename T, typename C>
struct IsPointerToMemberT<T C::*> : std::true_type {    // 偏特化
    using MemberT = T;
    using ClassT = C;
};
```

在这里，额外成员提供了成员的类型和该成员的类类型。

### 识别函数类型

```c++
#include "../typelist/typelist.hpp"

template <typename T>
struct IsFunctionT : std::false_type {                      // 基本模板：没有函数
};

template <typename R, typename... Params>
struct IsFunctionT<R (Params...)> : std::true_type {        // 函数
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = false;
};

template <typename R, typename... Params>
struct IsFunctionT<R (Params..., ...)> : std::true_type {   // 变参函数
    using Type = R;
    using ParamT = Typelist<Params...>;
    static constexpr bool variadic = true;
};
```

`IsFunctionT` 不处理所有的函数类型，因为函数类型可以有 `const` 和 `volatile` 限定符，以及 lvalue(`&`) 和 rvalue(`&&`) 引用限定符，并且还有 `noexcept` 限定符（在 C++17 中）。

标记为 `const` 的函数类型实际上不是 `const` 类型，因此 `RemoveConst` 无法从函数类型中分离 `const`。要识别具有限定符的函数类型，我们需要引入大量额外的偏特化，包括对于限定符的每一个组合（无论有没有包含 C 风格的可变参数）的偏特化。在这里，我们仅说明许多必需的偏特化中的 5 个。

```c++
template <typename R, typename... Params>
struct IsFunctionT<R, (Params...) const> : std::true_type {
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = false;
};

template <typename R, typename... Params>
struct IsFunctionT<R, (Params..., ...) volatile> : std::true_type {
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = true;
};

template <typename R, typename... Params>
struct IsFunctionT<R, (Params..., ...) const volatile> : std::true_type {
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = true;
};

template <typename R, typename... Params>
struct IsFunctionT<R, (Params..., ...) &> : std::true_type {
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = true;
};

template <typename R, typename... Params>
struct IsFunctionT<R, (Params..., ...) &&> : std::true_type {
    using Type = R;
    using ParamsT = Typelist<Params...>;
    static constexpr bool variadic = true;
};
```

### 确定类类型

和目前处理的其他复合类型不同，我们没有专门匹配类类型的偏特化模式。枚举所有类类型也不可行，因为其是基本类型。我们需要使用间接方法来标识类类型，方法是生成对所有类类型（而不是其他类型）都有效的类型或表达式。对于这种表达式，我们可以应用 SFINAE 特征技术。

在这种情况下，类类型最方便使用的属性是，只有类类型可以作为指向成员的指针的基础。也就是说，在 `X Y::*` 形式的类型构造中，`Y` 只能是类类型。下面的 `IsClassT<>` 表达式利用了此属性（为类型 `X` 任意选取 `int`）：

```c++
#include <type_traits>

template <typename T, typename = std::void_t<>>
struct IsClassT : std::false_type {     // 基本模板：默认情况下没有类
};

template <typename T>
struct IsClassT<T, std::void_t<int T::*>>   // 类可以有指向成员的指针
  : std::true_type {};
```

C++ 语言指定，lambda 表达式的类型是唯一的、未命名的非联合类类型。因此，当检查 lambda 表达式是否为类类型对象时，将产生 `true`。

还要注意，表达式 `int T::*` 对于联合类型（它们也是遵循 C++ 标准的类类型）也是有效的。

C++ 标准库提供了 `std::is_class<>` 和 `std::is_union<>` 这两个特征。然而，这些特征需要特殊的编译器支持，因为目前无法通过任何标准的核心语言技术来区分类 / 结构体类型与联合类型。

### 确定枚举类型

唯一没有被特征分类的类型是枚举类型。检查枚举类型可以直接通过编写基于 SFINAE 的特征来执行。该特征检查枚举类型是否显式转换为整型（比如 `int`），并显式排除基本类型、类类型、引用类型、指针类型和指向成员的指针类型，所有这些类型都可以转换为整型，但不是枚举类型。相反，我们只需注意，不属于任何其他类型的任何类型，都必须是一个枚举类型，我们可以实现如下：

```c++
template <typename T>
struct IsEnumT {
    static constexpr bool value = !IsFundaT<T>::value &&
                                  !IsPointerT<T>::value &&
                                  !IsReferenceT<T>::value &&
                                  !IsArrayT<T>::value &&
                                  !IsPointerToMemberT<T>::value &&
                                  !IsFunctionT<T>::value &&
                                  !IsClassT<T>::value;
};
```

C++ 标准库提供了 `std::is_enum<>` 特征。通常，为了提高编译性能，编译器将直接支持这种特征，而不是将其作为“其他任意东西”来实现。

## policy 特征

到目前为止，我们给出了特征模板被用来确定模板参数的属性的示例：这些参数表示什么类型，应用于该类型的值的运算符的结果类型等。这样的特征被称为 property 特征。

相比之下，一些特征定义了应该如何对待某些类型，我们称之为 policy 特征。这让人想起之前讨论的 policy 类的概念（我们已经指出，特征和 policy 之间的区别并不是很明显），但是 property 特征往往是与模板参数相关联的唯一属性（而 policy 类通常独立于其他模板参数）。

通常可以把 policy 特征实现为类型函数，而 policy 特征通常将 policy 封装在成员函数中。

### 只读的参数类型

```c++
// traits/rparam.hpp
#include "ifthenelse.hpp"
#include <type_traits>

template <typename T>
struct RParam {
  using Type
    = IfThenElse<(sizeof(T) <= 2 * sizeof(void*)
                    && std::is_trivially_copy_constructible<T>::value
                    && std::is_trivially_move_constructible<T>::value),
                  T,
                  T const&>;
};
```

```c++
// 允许以传值或传引用的方式传递参数的函数
template <typename T1, typename T2>
void foo(typename RParam<T1>::Type p1,
         typename RParam<T2>::Type p2) {
    ...
}
```

使用 `RParam<>` 有一些明显的缺点。首先，函数声明要复杂得多。其次，可能更令人反感的是，像 `foo()` 这样的函数不能用实参演绎来调用，因为模板参数只出现在函数参数的限定符里面。因此，调用的位置必须指定显式的模板实参。

对于上述问题，一个“笨拙”的解决方法是：使用提供完美转发的内联包装（wrapper）函数模板，但它假定编译器将忽略内联函数。

```c++
#include "rparam.hpp"
#include "rparamcls.hpp"

// 允许以传值或传引用的方式传递参数的函数
template <typename T1, typename T2>
void foo_core(typename RParam<T1>::Type p1,
              typename RParam<T2>::Type p2) {
    ...
}

// 为了避免指定显式模板参数而实现的包装
template <typename T1, typename T2>
void foo(T1&& p1, T2&& p2) {
    foo_core<T1, T2>(std::forward<T1>(p1), std::forward<T2>(p2));
}

int main() {
    MyClass1 mc1;
    MyClass2 mc2;
    foo(mc1, mc2);  // 等价于 foo_core<MyClass1, MyClass2>(mc1, mc2)
}
```

## 在标准库中

在 C++11 中，类型特征成为 C++ 标准库的一个固定部分。它们或多或少包含本章讨论的类型函数和类型特征。然而，对于其中的一些问题，如琐碎的操作检测特征和 `std::is_union`，还没有 C++ 语言解决方案。编译器为这些特征提供了内在的支持。而且，即使存在缩短编译时间的语言内解决方案，编译器也开始支持特征。

因此，如果需要类型特征，建议在可用时使用 C++ 标准库中的那些类型。

C++ 标准库还定义了一些 policy 特征和 property 特征，如下所示。

- 类模板 `std::char_traits` 被 `string` 和输入输出流类用作 policy 特征函数。
- 为了使算法易于适应所使用的标准迭代器的类型，C++ 标准库提供了一个非常简单的 `std::iterator_traits` 属性模板（并用于标准库接口）。
- 模板 `std::numeric_limits` 也可以用作特征模板。
- 标准容器类型的内存分配是使用 policy 特征类来处理的。

自 C++98 以来，模板 `std::allocator` 被用作这些目的的标准组件。C++11 添加了模板 `std::allocator_traits`，以改变分配器的 policy 或行为（在经典行为和作用域内的分配器之间切换，分配器没有包含在 C++11 之前的框架里）。
