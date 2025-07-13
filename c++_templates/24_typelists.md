# 类型列表

卓有成效的编程通常需要使用各种数据结构，元编程也不例外。对于类型元编程来说，核心数据结构是类型列表（typelist），正如它的名字所示，它是一个包含类型的列表。模板元编程可以对这些类型列表进行运算，操纵它们来最终产生可执行程序的一部分。在本章中，我们将讨论使用类型列表的技巧。

## 解剖一个类型列表

类型列表是一种表示类型的列表的类型，可以由模板元程序来操纵。它提供了列表相关的常见操作：迭代列表中的元素（类型）、添加元素或删除元素。然而，类型列表与大多数运行期数据结构（如 `std::list`）不同，因为它们不允许“变异”。例如，向 `std::list` 添加一个元素会改变列表本身，而这种改变可以被程序中任何可以访问该列表的其他部分所观察。而向类型列表添加一个元素并不会改变原来的类型列表：向现有的类型列表添加一个元素只会创建一个新的类型列表而不会修改原来的类型列表。

一个类型列表通常被实现为一个类模板的特化，在其模板实参中编码类型列表的内容，即它所包含的类型和这些类型的顺序。以下类型列表的直接实现就将其元素编码进了一个形参包：

```c++
template <typename... Elements>
class Typelist {};
```

`Typelist` 的元素被直接写成其模板实参。一个空的类型列表被写成 `Typelist<>`，一个只包含 `int` 类型元素的类型列表被写成 `Typelist<int>`，以此类推。下面是一个包含所有有符号整型元素的类型列表：

```c++
using SignedIntegralTypes = Typelist<signed char, short, int, long, long long>;
```

操纵这个类型列表通常需要将类型列表分成多个部分，一般是将列表中的第 1 个元素（头）与列表中的其余元素（尾）分开。例如，`Front` 元函数从类型列表中提取第 1 个元素：

```c++
template <typename List>
class FrontT;

template <typename Head, typename... Tail>
class FrontT<Typelist<Head, Tail...>> {
  public:
    using Type = Head;
};

template <typename List>
using Front = typename FrontT<List>::Type;
```

因此，`FrontT<SignedIntegralTypes>::Type`（更简洁地写成 `Front<SignedIntegralTypes>`）将给出 `signed char`。类似地，`PopFront` 元函数从类型列表中删除第 1 个元素。它的实现将类型列表元素分成头和尾，然后用尾中的元素形成一个新的 `Typelist` 特化。

```c++
template <typename List>
class PopFrontT;

template <typename Head, typename... Tail>
class PopFrontT<Typelist<Head, Tail...>> {
  public:
    using Type = Typelist<Tail...>;
};

template <typename List>
using PopFront = typename PopFrontT<List>::Type;
```

`PopFront<SignedIntegralTypes` 生成如下类型列表：

```c++
Typelist<short, int, long, long long>
```

也可以将元素插入类型列表的前面，做法是将所有现有的元素捕捉到一个模板形参包中，然后创建一个包含所有这些元素的新的 `Typelist` 特化。

```c++
template<typename List, typename NewElement>
class PushFrontT;

template<typename... Elements, typename NewElement>
class PushFrontT<Typelist<Elements...>, NewElement> {
  public:
    using Type = Typelist<NewElement, Elements...>;
};

template<typename List, typename NewElement>
using PushFront = typename PushFrontT<List, NewElement>::Type;
```

不难想到，

```c++
PushFront<SignedIntegralTypes, bool>
```

给出：

```c++
Typelist<bool, signed char, short, int, long, long long>
```

## 类型列表算法

类型列表的基本操作 `Front`、`PopFront` 和 `PushFront` 可以组合在一起以创建更有趣的类型列表操作。例如，可以通过对 `PopFront` 的结果应用 `PushFront` 来替换类型列表中的第 1 个元素。

```c++
using Type = PushFront<PopFront<SignedIntegralTypes>, bool>;
// 相当于 Typelist<bool, short, int, long, long long>
```

进一步，可以实现算法（如搜索、转换、反转）作为操作类型列表的模板元函数。

### 索引

类型列表最基本的操作之一是从列表中提取一个特定的元素。24.1 节说明了如何实现一个提取第 1 个元素的操作。在这里把这个操作推广到提取第 N 个元素。例如，为了提取给定类型列表中索引为 2 的类型元素，可以这样写：

```c++
using TL = NthElement<Typelist<short, int, long>, 2>;
```

这让 `TL` 成为 `long` 的别名。`NthElement` 运算由一个递归的元程序来实现，它遍历类型列表直到找到要找的元素：

```c++
// 递归分支
template <typename List, unsigned N>
class NthElementT : public NthElementT<PopFront<List>, N-1> {};

// 基础分支
template <typename List>
class NthElementT<List, 0> : public FrontT<List> {};

template <typename List, unsigned N>
using NthElement = typename NthElementT<List, N>::Type;
```

首先，考虑基础分支，由 `N` 为 0 的偏特化处理。该偏特化通过提供列表前面的元素来终止递归，它通过公开继承 `Front<List>` 并使用元函数转发来做到这一点。`FrontT<List>`（间接地）提供了 `Type` 类型的别名，该别名既是这个列表前面的元素，也是 `NthElement` 元函数的结果。

递归分支也是模板的主要定义，它遍历类型列表。因为偏特化保证了 `N` 大于 0，递归分支从列表中删除前面的元素，并从剩余的列表中请求第 `N-1` 个元素。在我们的例子里：

```c++
NthElementT<Typelist<short, int, long>, 2>
```

继承自

```c++
NthElementT<Typelist<int, long>, 1>
```

以上又继承自

```c++
NthElementT<Typelist<long>, 0>
```

这样我们就遇到了基础分支，它继承自 `FrontT<Typelist<long>>` 并通过嵌套的类型 `Type` 提供了结果。

### 寻找最佳匹配

许多类型列表算法在类型列表中搜索数据。例如，人们可能想在类型列表中找到最大的类型。这也可以通过递归模板元程序来完成：

```c++
template <typename List>
class LargestTypeT;

// 递归分支
template <typename List>
class LargestTypeT {
  private:
    using First = Front<List>;
    using Rest = typename LargestTypeT<PopFront<List>>::Type;
  public:
    using Type = IfThenElse<(sizeof(First) >= sizeof(Rest)), First, Rest>;
};

// 基础分支
template<>
class LargestTypeT<Typelist<>> {
  public:
    using Type = char;
};

template <typename List>
using LargestType = typename LargestTypeT<List>::Type;
```

`LargestType` 算法将返回类型列表中第 1 次出现的最大的类型。例如，如果给定类型列表 `Typelist<bool, int, long, short>`，该算法将返回与 `long` 相同大小的第 1 个类型，可能是 `int` 或 `long`，具体取决于你的平台。

`LargestTypeT` 的基本模板也是算法的递归分支。它采用了常见的第 1 个 / 其余惯用法（first/rest idiom），分 3 步。首先，它只根据第 1 个元素计算出了一个部分结果，在这种情况下，第 1 个元素也就是列表的前部元素，将其放在 `First` 中。其次，它递归计算列表中其他元素的结果，并将该结果放在 Rest 中。例如，在类型列表 `Typelist<bool, int, long, short>` 的第一步递归中，First 是 `bool`，而 Rest 是对 `Typelist<int, long, short>` 应用该算法的结果。最后，将 First 和 Rest 的结果结合起来，给出最终结果。在这里，`IfThenElse` 选择列表中的第 1 个元素（First）或到目前为止的最佳候选元素（Rest）中较大的一个，并返回“赢家”。`>=` 会打破平局，优先选择在列表前面的元素。

递归在列表为空时终止。默认情况下，我们使用 `char` 作为哨兵类型以初始化算法，因为每个类型至少和 `char` 一样大。

请注意，基础分支明确提到了空类型列表 `Typelist<>`。这有点遗憾，因为它排除了其他形式的类型列表的使用，我们将在后面的章节（包括 24.3 节、24.5 节和第 25 章）中回到这一话题。为了解决此问题，我们引入一个 `IsEmpty` 元函数，用来确定给定的类型列表是否没有元素：

```c++
template <typename List>
class IsEmpty {
  public:
    static constexpr bool value = false;
};

template <>
class IsEmpty<Typelist<>> {
  public:
    static constexpr bool value = true;
};
```

使用 `IsEmpty`，我们可以实现 `LargestType`，使其对任何实现了 `Front`、`PopFront` 和 `IsEmpty` 的类型列表都适用。如下所示：

```c++
template <typename List, bool Empty = IsEmpty = IsEmpty<List>::value>
class LargestTypeT;

// 递归分支
template <typename List>
class LargestTypeT<List, false> {
  private:
    using Contender = Front<List>;
    using Best = typename LargestTypeT<PopFront<List>>::Type;
  public:
    using Type = IfThenElse<(sizeof(Contender) >= sizeof(Best)), Contender, Best>;
};

// 基础分支
template <typename List>
class LargestTypeT<List, true> {
  public:
    using Type = char;
};

template <typename List>
using LargestType = typename LargestTypeT<List>::Type;
```

`LargestTypeT` 默认的第 2 个模板参数 `Empty`，用于检查列表是否为空。如果不为空，递归分支（将此实参固定为 `false`）继续搜索列表。否则，基础分支（将此实参固定为 `true`）终止递归并提供初始结果（`char`）。

### 追加元素到类型列表

`PushFront` 原语运算允许我们在类型列表的前面添加一个新元素，以产生一个新的类型列表。假设我们还希望能在列表的末尾添加一个新元素，像我们在运行期容器（如 `std::list` 和 `std::vector`）中经常做的那样。对于我们的 `Typelist` 模板，只需要对 24.1 节中的 `PushFront` 实现做一个小改动就可以实现 `PushBack`：

```c++
template <typename List, typename NewElement>
class PushBackT;

template <typename... Elements, typename NewElement>
class PUshBackT<Typelist<Elements...>, NewElement> {
  public:
    using Type = Typelist<Elements..., NewElement>;
};

template <typename List, typename NewElement>
using PushBack = typename PushBackT<List, NewElement>::Type;
```

不过，像 `LargestType` 算法一样，可以只使用 `Front`、`PushFront`、`PopFront` 和 `IsEmpty` 等原语运算来实现 `PushBack` 的通用算法：

```c++
template <typename List, typename NewElement, bool = IsEmpty<List>::value>
class PushBackRecT;

// 递归分支
template <typename List, typename NewElement>
class PushBackRecT<List, NewElement, false> {
  using Head = Front<List>;
  using Tail = PopFront<List>;
  using NewTail = typename PushBackRecT<Tail, NewElement>::Type;

 public:
  using Type = PushFront<Head, NewTail>;
};

// 基础分支
template <typename List, typename NewElement>
class PushBackRecT<List, NewElement, true> {
  public:
    using Type = PushFront<List, NewElement>;
};

// 泛型的尾部推入运算
template <typename List, typename NewElement>
class PushBackT : public PushBackRecT<List, NewElement> {};

template <typename List, typename NewElement>
using PushBack = typename PushBackT<List, NewElement>::Type;
```

`PushBackRecT` 模板管理递归。在基础分支中，我们使用 `PushFront` 将 `NewElement` 添加到空列表中，因为对于空列表，`PushFront` 和 `PushBack` 等效。递归分支有趣得多。它将列表分割成它的第 1 个元素（`Head`）和一个包含其余元素的类型列表（`Tail`）。然后，新元素被递归地追加到尾部，以产生 `NewTail`。然后我们再次使用 `PushFront` 将 `Head` 添加到列表 `NewTail` 的前面，形成最终的列表。

这一通用的 `PushBackRecT` 实现适用于各种各样的类型列表。像本节前面展现的几种算法一样，它需要线性数量的模板实例化来求值，因为对于一个长度为 N 的类型列表，将有 N+1 个 `PushBackRecT` 和 `PushFrontT` 的实例化，以及 N 个 `FrontT` 和 `PopFrontT` 实例。通过统计模板实例化的数量，可以粗略估计编译一个特定元程序所需的时间，因为模板实例化本身对编译器来说是一个相当复杂的过程。

编译时间对于大型模板元程序来说可能是一个问题，所以尝试减少这些算法所执行的模板实例化的数量是合理的。事实上，我们对 `PushBack` 的第 1 个实现（采用了 `Typelist` 的偏特化）只需要恒定数量的模板实例化，这让它的效率（在编译期）远高于泛化版本的效率。此外，因为它被描述为 `PushBackT` 的偏特化，所以当在 `Typelist` 实例上执行 `PushBack` 时，这个高效的实现会被自动选择，这就把算法特化的概念带到了模板元程序中。20.1 节中讨论的许多技巧都可以应用于模板元程序，以减少算法所需的模板实例化的数量。

### 反转类型列表

当类型列表的元素依从某种顺序排列的时候，在应用某些算法时，反转类型列表中元素的顺序将更为方便。`Reverse` 算法实现了这个元函数。

```c++
template <typename List, bool Empty = IsEmpty<List>::value>
class ReverseT;

template <typename List>
using Reverse = typename ReverseT<List>::Type;

// 递归分支
template <typename List>
class ReverseT<List, false> : public PushBackT<Reverse<PopFront<List>>, Front<List>> {};

// 基础分支
template <typename List>
class ReverseT<List, true> {
 public:
  using Type = List;
};
```

这个元函数递归的基础分支是空类型列表上的恒等函数。递归分支将列表分割成第 1 个元素和其余元素。

`Reverse` 算法让类型列表的 `PopBackT` 实现成为可能，它可以删除类型列表的最后一个元素：

```c++
template <typename List>
class PopBackT {
 public:
  using Type = Reverse<PopFront<Reverse<List>>>;
};

template <typename List>
using PopBack = typename PopBackT<List>::Type;
```

该算法反转列表、删除反转后列表的第 1 个元素（使用 `PopFront`），再反转结果列表。

### 转化类型列表

我们可能希望以某种方式“转化”类型列表中的所有类型。例如通过使用 `AddConst` 元函数将每个类型转化为其有 `const` 限定的变体。

```c++
template <typename T>
struct AddConstT {
  using Type = T const;
};

template <typename T>
using AddConst = typename AddConstT<T>::Type;
```

为了达到这个目的，我们将实现一个 `Transform` 算法，该算法接收一个类型列表和一个元函数，产出另一个类型列表，其中包含的是对原来的类型列表中每个类型应用元函数的结果。例如，类型

```c++
Transform<SignedIntegralTypes, AddConstT>
```

通常是一个类型列表，其中包含 `signed char const`、`short const`、`int const`、`long const` 以及 `long long const`。元函数是通过一个模板的模板形参提供的，它将一个输入类型映射到一个输出类型。不难想到，`Transform` 算法本身仍然是递归算法：

```c++
template <typename List, template <typename T> class MetaFun, bool Empty = IsEmpty<List>::value>
class TransformT;

// 递归分支
template <typename List, template <typename T> class MetaFun>
class TransformT<List, MetaFun, false> : public PushFrontT<typename TransformT<PopFront<List>, MetaFun>::Type,
                                                           typename MetaFun<Front<List>>::Type> {};

// 基础分支
template <typename List, template <typename T> class MetaFun>
class TransformT<List, MetaFun, true> {
 public:
  using Type = List;
};

template <typename List, template <typename T> class MetaFun>
using Transform = typename TransformT<List, MetaFun>::Type;
```

尽管这里的递归分支在语法上笨重了些，但直截了当。转化的结果是转化类型列表中的第 1 个元素（`PushFront` 的第 2 个实参），并将其添加到通过递归转化类型列表中的其余元素（`PushFront` 的第 1 个实参）生成的序列的开头的结果。

另见 24.4 节，该节展示如何开发一个更有效的 `Transform` 实现。

### 累加类型列表

`Transform` 是一种有用的算法，用于转化序列中的每个元素。它经常与 `Accumulate` 一起使用，`Accumulate` 将一个序列的所有元素组合成一个单一的结果值。`Accumulate` 算法接收一个带有元素 `T1`, `T2`, ..., `TN` 的类型列表 `T`、一个初始类型 `I` 和一个元函数 `F`，它接收两个类型并返回一个类型。它返回 `F(F(F(...F(I, T1), T2), ..., TN-1), TN)`，其中在累加的第 i 步，`F` 被应用于前 i-1 步的结果和 `Ti` 上。

根据类型列表、`F` 的选择和初始类型，我们可以使用 `Accumulate` 产生许多不同的结果。例如，如果 `F` 选择两个类型中的最大一个，`Accumulate` 将表现得像 `LargestType` 算法。而如果 `F` 接收一个类型列表和一个类型，并把类型推入类型列表的后面，`Accumulate` 就会表现得像 `Reserve` 算法。

`Accumulate` 的实现遵循了我们的标准递归-元程序组成方式。

```c++
template <typename List,
          template<typename X, typename Y> class F,
          typename I,
          bool = IsEmpty<List>::value>
class AccumulateT;

// 递归分支
template <typename List,
          template <typename X, typename Y> class F,
          typename I>
class AccumulateT<List, F, I, false>
  : public AccumulateT<PopFront<List>, F, typename F<I, Front<List>>::Type> {};

// 基础分支
template <typename List,
          template <typename X, typename Y> class F,
          typename I>
class AccumulateT<List, F, I, true> {
 public:
  using Type = I;
};

template <typename Lists,
          template <typename X, typename Y> class F,
          typename I>
using Accumulate = typename AccumulateT<List, F, I>::Type;
```

这里，初始类型 `I` 也被用作累加器，用于捕获当前的结果。于是，基础分支在到达类型列表结尾的时候返回这个结果。在递归分支中，算法将 `F` 应用与先前的结果（`I`）和列表的首元素，将应用 `F` 的结果传递下去，将其作为初始类型继续累加列表的剩余部分。

有了 `Accumulate`，就可以通过将 `PushFrontT` 作为元函数 `F`，并用一个空类型列表（`TypeList<T>`）作为初始类型 `I` 来反转一个类型列表：

```c++
using Result = Accumulate<SignedIntegralTypes, PushFrontT, Typelist<>>;
// 得到 Typelist<long long, long, int, short, signed char>
```

实现基于累加器的 `LargestType`，即 `LargestTypeAcc`，需要多费些功夫，因为我们需要给出一个返回两个类型中较大者的元函数：

```c++
template <typename T, typename U>
class LargestTypeT
  : public IfThenElseT<sizeof(T) >= sizeof(U), T, U> {};

template <typename Typelist>
class LargestTypeAccT
  : public AccumulateT<PopFront<Typelist>, LargerTypeT, Front<Typelist>> {};

template <typename Typelist>
using LargestTypeAcc = typename LargestTypeAccT<TypeList>::Type;
```

请注意，`LargestType` 的这种写法需要一个非空的类型列表，因为它将类型列表的第 1 个元素作为初始类型。可以显式地处理空列表的情况，要么返回一些哨兵类型（`char` 或 `void`），要么将算法本身写成对 SFINAE 友好的，正如 19.4.4 节中所讨论的：

```c++
template <typename T, typename U>
class LargerTypeT
  : public IfThenElseT<sizeof(T) >= sizeof(U), T, U> {};

template <typename Typelist, bool = IsEmpty<Typelist>::value>
class LargestTypeAccT;

template <typename Typelist>
class LargestTypeAccT<Typelist, false>
  : public AccumulateT<PopFront<Typelist>, LargerTypeT, Front<Typelist>> {};

template <typename Typelist>
class LargestTypeAccT<Typelist, true> {};

template <typename Typelist>
using LargestTypeAcc = typename LargestTypeAccT<Typelist>::Type;
```

`Accumulate` 是一种强大的类型列表算法，因为它允许我们表达许多不同的运算，所以它可以被视为一种操纵类型列表的基础算法。

### 插入排序

我们来实现一个插入排序。与其他算法一样，递归步骤将列表分割成第 1 个元素（头部）和其余元素（尾部）。然后对尾部进行（递归）排序，并将头部插入排序后的列表中的正确位置。这个算法的框架还是以类型列表算法表达。

```c++
template <typename List,
          template <typename T, typename U> class Compare,
          bool = IsEmpty<List>::value>
class InsertionSortT;

template <typename List,
          template <typename T, typename U> class Compare>
using InsertionSort = typename InsertionSortT<List, Compare>::Type;

// 递归分支（将第 1 个元素插入排好序的列表）
template <typename List,
          template <typename T, typename U> class Compare>
class InsertionSortT<List, Compare, false>
  : public InsertionSortT<InsertionSort<PopFront<List>, Compare>, Front<List>, Compare> {};

// 基础分支（空的列表被排序）
template<typename List,
         template<typename T, typename U> class Compare>
class InsertionSortT<List, Compare, true> {
 public:
  using Type = List;
};
```

参数 `Compare` 是类型列表中排列元素时用于比较的类。它接收两种类型，求值为一个布尔值，并赋给它的 `value` 成员。基础分支处理空的类型列表，可以说是小菜一碟。

插入排序的核心是 `InsertSortedT` 函数，它将值插入已排序列表中，其位置是仍能保持列表有序的第 1 个位置：

```c++
#include "identity.hpp"

template<typename List, typename Element,
        template<typename T, typename U> class Compare,
        bool = IsEmpty<List>::value>
class InsertSortedT;

// 递归分支
template<typename List, typename Element,
        template<typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, false> {
  // 计算结果列表的尾部
  using NewTail =
  typename IfThenElse<Compare<Element, Front<List>>::value,
                      IdentityT<List>,
                      InsertSortedT<PopFront<List>, Element, Compare>
           >::Type;
  // 计算结果列表的头部
  using NewHead = IfThenElse<Compare<Element, Front<List>>::value,
                             Element,
                             Front<List>>;
 public:
  using Type = PushFront<NewTail, NewHead>;
};

// 基础分支
template<typename List, typename Element,
         template<typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, true>
  : public PushFrontT<List, Element> {};

template<typename List, typename Element,
         template<typename T, typename U> class Compare>
using InsertSorted = typename InsertSortedT<List, Element, Compare>::Type;
```

基础分支不必多说，因为单元素列表总是有序的。递归分支的不同在于要插入的元素是在列表的头部还是在列表的尾部。如果插入的元素在已经排好序的列表中的第 1 个元素之前，用 `PushFront` 将该元素从前面加入列表中。否则，我们将列表分成头部和尾部进行递归，从而将该元素插入尾部，然后将头部添加到将元素插入尾部的结果中。

这个实现包括一个编译期的优化，以避免实例化那些不会被使用的类型，这个技巧在 19.7.1 节中讨论过。下面的实现在技术上也是正确的：

```c++
template <typename List, typename Element,
          template <typename T, typename U> class Compare>
class InsertSortedT<List, Element, Compare, false>
 : public IfThenElseT<Compare<Element, Front<List>>::value,
                      PushFront<List, Element>,
                      PushFront<InsertSorted<PopFront<List>,
                                             Element, Compare>,
                                Front<List>>> {};
```

然而，这种递归分治的写法不必要地降低了效率，因为它在 `IfThenElseT` 的两个分支中都对模板实参求值，尽管只会使用一个分支。在我们的例子中，在 then 分支中的 `PushFront` 通常是相当 “便宜的”，但在 else 分支中递归的 `InsertSorted` 调用则不然。

在优化实现中，第 1 个 `IfThenElse` 计算结果列表的尾部 `NewTail`。`IfThenElse` 的第 2 个和第 3 个参数都是计算各分支结果的元函数。第 2 个参数（then 分支）使用 `IdentityT` 来产生未修改的 `List`。第 3 个参数（else 分支）使用 `InsertSortedT` 来计算在已排序列表的后面插入元素的结果。在顶层，`IdentityT` 或 `InsertSortedT` 只有其中之一会被实例化，所以很少有额外的工作被执行（在更差的情况下，是 `PopFront`）。第 2 个 `IfThenElse` 则会计算出结果列表的头部，分支会被立即求值，因为这两个分支都被认为是“便宜”的。最后的列表是由计算出的 `NewHead` 和 `NewTail` 构建的。这种写法有一个理想的特性，即在排序的列表中插入一个元素所需的实例化数量与它在结果列表中的位置成正比。这表现为插入排序的一个更高层次的属性，即对一个已经排序的列表进行排序的实例化数量与列表的长度呈线性关系。（对于反向排序的输入，输入排序的实例化数量与列表长度成平方关系）。

## 非类型类型列表

类型列表以一套丰富的算法和运算提供了描述和操纵类型序列的功能。在某些情况下，可以用它处理编译期的数值序列，如多维数组的边界或另一个类型列表的索引。

有多种方法可以产生一个编译期值的类型列表。一种简单的方法是定义一个 `CTValue`（compile-time value 的缩写）类模板，代表类型列表中某个特定类型的值：

```c++
template<typename T, T Value>
struct CTValue {
  static constexpr T value = Value;
};
```

通过 `CTValue` 模板，我们现在可以表达一个包含前几个素数的整数值的类型列表：

```c++
using Primes = Typelist<CTValue<int, 2>, CTValue<int, 3>,
                        CTValue<int, 5>, CTValue<int, 7>,
                        CTValue<int, 11>>;
```

有了这种表示，可以对值组成的列表进行数值计算，比如计算这些素数的乘积。

首先 `MultiplyT` 模板接收两个相同类型的编译期值，将输入值相乘，从而产生一个相同类型的新编译期值：

```c++
template <typename T, typename U>
struct MultiplyT;

template <typename T, T Value1, T Value2>
struct MultiplyT<CTValue<T, Value1>, CTValue<T, Value2>> {
 public:
  using Type = CTValue<T, Value1 * Value2>;
};

template<typename T, typename U>
using Multiply = typename MultiplyT<T, U>::Type;
```

然后，通过使用 `MultiplyT`，以下表达式给出 `Primes` 中所有素数的乘积。

```c++
Accumulate<Primes, MultiplyT, CTValue<int, 1>>::value
```

遗憾的是，`Typelist` 和 `CTValue` 的这种用法相对繁琐啊啊，尤其是在所有的值都是同一类型的情况下。我们可以引入一个别名模板 `CTTypelist` 来优化这种特殊情况，它提供了一个同质的值的列表，用 `CTValue` 的 `Typelist` 来描述。

```c++
template<typename T, T... Values>
using CTTypelist = Typelist<CTValue<T, Values>...>;
```

现在可以用 `CTTypelist` 写一个等价（但简介得多）的 `Primes` 的定义：

```c++
using Primes = CTTypelist<int, 2, 3, 5, 7, 11>;
```

该定义的唯一缺点是，别名模板终究只是别名，因此错误信息可能最终会输出底层的 `CTValueType` 类型的 `Typelist`，这会带来冗长的错误信息。为解决这个问题，可以创建一个全新的类型列表类 `Valuelist`，直接存储这些值。

```c++
template<typename T, T... Values>
struct Valuelist {};

template<typename T, T... Values>
struct IsEmpty<Valuelist<T, Values...>> {
  static constexpr bool value = sizeof...(Values) == 0;
};

template<typename T, T Head, T... Tail>
struct FrontT<Valuelist<T, Head, Tail...>> {
  using Type = CTValue<T, Head>;
  static constexpr T value = Head;
};

template<typename T, T Head, T... Tail>
  struct PopFrontT<Valuelist<T, Head, Tail...>> {
  using Type = Valuelist<T, Tail...>;
};

template<typename T, T... Values, T New>
  struct PushFrontT<Valuelist<T, Values...>, CTValue<T, New>> {
  using Type = Valuelist<T, New, Values...>;
};

template<typename T, T... Values, T New>
struct PushBackT<Valuelist<T, Values...>, CTValue<T, New>> {
  using Type = Valuelist<T, Values..., New>;
};
```

通过提供 `IsEmpty`、`FrontT`、`PopFrontT` 和 `PushFrontT`，我们使 `Valuelist` 成为一个合格的类型列表，可以与本章中定义的算法一起使用。`PushBackT` 以算法特化的形式提供，以降低其在编译期的成本。`Valuelist` 可以与之前定义的 `InsertionSort` 算法一起使用，例如：

```c++
template <typename T, typename U>
struct GreaterThanT;

template <typename T, T First, T Second>
struct GreaterThanT<CTValue<T, First>, CTValue<T, Second>> {
  static constexpr bool value = First > Second;
};

void valuelisttest() {
  using Integers = Valuelist<int, 6, 2, 4, 9, 5, 2, 1, 7>;

  using SortedIntegers = InsertionSort<Integers, GreaterThanT>;

  static_assert(std::is_same_v<SortedIntegers,
                               Valuelist<int, 9, 7, 6, 5, 4, 2, 2, 1>>,
                "insertion sort failed");
}
```

请注意，可以通过使用字面量运算符提供初始化 `CTValue` 的功能，例如：

```c++
auto a = 42_c;  // 初始化 a 为 CTValue<int, 42>
```

### 可推导非类型实参

在 C++17 中，`CTValue` 可以通过使用一个单一的、可推导的非类型形参（用 `auto` 标识）来改进：

```c++
template<auto Value>
struct CTValue {
 static constexpr auto value = Value;
};
```

这样就不需要再每次使用 `CTValue` 时都指定类型，使其更易用：

```c++
using Primes = Typelist<CTValue<2>, CTValue<3>, CTValue<5>,
                        CTValue<7>, CTValue<11>>;
```

C++17 的 `Valuelist` 也可以这样做，但结果不一定更好。正如 15.10.1 节中所指出的，一个具有推导类型的非类型形参包允许每个实参的类型不同：

```c++
template<auto... Values>
class Valuelist {};

int x;
using MyValueList = Valuelist<1, 'a', true, &x>;
```

虽然这样的异质的值列表可能有用，但它与我们以前的 `Valuelist` 不一样，后者要求所有的元素都有相同的类型。尽管编程者可以要求所有的元素都有相同的类型（15.10.1 节也有讨论），一个空的 `Valuelist<>` 必然没有已知的元素类型。

## 使用包扩展来优化算法

包扩展可以作为一种有用的机制，将类型列表迭代的工作转交给编译器。24.2.5 节中开发的 `Transform` 算法是一个适于使用包扩展的算法，因为它对列表中的每个元素都进行了同样的运算。这样就可以对一个 `Typelist` 的 `Transform` 进行算法特化（通过偏特化）：

```c++
template<typename... Elements, template<typename T> class MetaFun>
class TransformT<Typelist<Elements...>, MetaFun, false> {
 public:
  using Type = Typelist<typename MetaFun<Elements>::Type...>;
};
```

这个实现将类型列表的元素捕获到一个形参包 `Elements` 中。然后，它采用了一个具有 `typename MetaFun<Elements>::Type` 模式的包扩展，将元函数应用于 `Elements` 中的每个类型，并根据结果形成一个类型列表。这个实现可以说是比较简单的，因为它不需要递归，并且以一种相当直接的方式使用语言特性。此外，它需要更少的模板实例化，因为只需要实例化一个 `Transform` 模板的实例。`Transform` 算法仍然需要线性数量的 `MetaFun` 实例化，但这些实例化是该算法的根本。

其他算法也间接地从使用包扩展中受益。例如，24.2.4 节描述的 `Reverse` 算法需要线性数量的 `PushBack` 的实例化。通过 24.2.3 节描述的 `PushBack` 在 `Typelist` 上的包扩展形式（它仅需要一个实例化），可以知道 `Reverse` 仍是线性的算法。然而，该节描述的 `Reverse` 的更一般的递归实现本身在实例化的数量上是线性的，这就使得 `Reverse` 成了平方的算法。

包扩展也可以用来选择给定索引列表中的元素，以产生一个新的类型列表。`Select` 元函数接收一个类型列表和一个包含该类型列表索引的 `Valuelist`，然后产生一个包含 `Valuelist` 指定元素的新的类型列表。

```c++
template<typename Types, typename Indices>
class SelectT;

template<typename Types, unsigned... Indices>
class SelectT<Types, Valuelist<unsigned, Indices...>> {
 public:
  using Type = Typelist<NthElement<Types, Indices>...>;
};

template<typename Types, typename Indices>
using Select = typename SelectT<Types, Indices>::Type;
```

索引被捕获在形参包 `Indices` 中，该形参包扩展后产生一个 `NthElement` 类型的序列，以索引到给定的类型列表，将结果捕获在一个新的 `Typelist` 中。下面的例子说明了我们如何使用 `Select` 来反转一个类型列表。

```c++
using SignedIntegralTypes =
  Typelist<signed char, short, int, long, long long>;

using ReversedSignedIntegralTypes =
  Select<SignedIntegralTypes, Valuelist<unsigned, 4, 3, 2, 1, 0>>;
// 产生 Typelist<long long, long, int, short, signed char>
```

包含另一列表的索引的非类型类型列表通常被称为索引列表（或索引序列），它可以简化或消除递归计算。在 25.3.4 节中对索引列表有详细描述。

## cons 风格的类型列表

cons 风格类型列表可以表达与本章中描述的变参类型列表相同的所有算法。的确，许多描述过的算法正是使用和操纵与 cons 风格类型列表相同的风格来写的。然而，它们的一些缺点让我们更喜欢变参版本。首先，嵌套使得长的 cons 风格的类型列表在源代码和编译器诊断中难写难读。其次，一些算法（包括 `PushBack` 和 `Transform`）可以为变参类型列表特化，以提供更高效的实现（以实例化的数目衡量）。最后，为类型列表使用变参模板会良好地适应第 25 章和第 26 章讨论的异质容器上对变参模板的使用。
