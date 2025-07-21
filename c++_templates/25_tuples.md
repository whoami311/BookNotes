# 元组

元组以类似于类和结构体的方式聚集数据。例如，包含一个 `int`、一个 `double` 和 一个 `std::string` 成员的元组与一个包含 `int`、`double` 和 `std::string` 成员的结构体类似，只是元组的元素是通过位置（如 `0`、`1`、`2`）指代而不是通过名字指代。位置接口以及从类型列表中轻松构造元组的能力使元组比结构体更适于使用模板元编程技巧。

另一种看法是把元组看做可执行程序中类型列表的一种表现。例如，类型列表 `Typelist<int, double, std::string` 描述了可在编译期进行操作的包含 `int`、`double` 和 `std::string` 的类型序列，而 `Tuple<int, double, std::string>` 描述了可以在运行期进行操作的 `int`、`double` 和 `std::string` 的存储。

使用模板元编程与类型列表来生成可用于存储数据的元组是常见做法。例如，尽管我们在上面的例子中随意选择了 `int`、`double` 和 `std::string` 作为元素类型，但我们本来也可以用元程序来创建由元组存储的类型集合。

在本章的其余部分，我们将探讨 `Tuple` 类模板的实现和操作，它是类模板 `std::tuple` 的简化版本。

## 基础元组设计

元组包含对模板实参列表中每个类型的存储。这些存储可以通过函数模板 `get` 来访问，对元组 `t` 来说，写为 `get<I>(t)`。

元组存储的递归写法基于这样的思想：一个包含 N（N 大于 0）个元素的元组可以存储为一个元素（第 1 个元素，或者说是列表的头部）加上一个包含 N-1 个元素的元组（尾部），而包含 0 个元素的元组是单独的特殊情况。事实上，这与类型列表算法的泛型版本中使用的递归分解是一样的，而递归元组存储的实际实现也类似地展开：

```c++
template<typename... Types>
class Tuple;

// 递归分支
template<typename Head, typename... Tail>
class Tuple<Head, Tail...> {
 private:
  Head head;
  Tuple<Tail...> tail;
 public:
  // 构造函数
  Tuple() {}
  Tuple(Head const& head, Tuple<Tail...> const& tail)
    : head(head), tail(tail) {}
  // ...

  Head& getHead() { return head; }
  Head const& getHead() const { return head; }
  Tuple<Tail...>& getTail() { return tail; }
  Tuple<Tail...> const& getTail() const { return tail; }
};

// 基础分支
template<>
class Tuple<> {
  // 无须存储
};
```

在递归分支里，每个 `Tuple` 实力包含数据成员 `head`，用来存储列表中的第 1 个元素，以及数据成员 `tail`，用来存储列表中的其余元素。基础分支就是空元组，它没有相关联的存储。

`get` 函数模板遍历这个递归结构，以提取所请求的元素。

```c++
// 递归分支
template<unsigned N>
struct TupleGet {
  template<typename Head, typename... Tail>
  static auto apply(Tuple<Head, Tail...> const& t) {
    return TupleGet<N-1>::apply(t.getTail());
  }
};

// 基础分支
template<>
struct TupleGet<0> {
  template<typename Head, typename... Tail>
  static Head const& apply(Tuple<Head, Tail...> const& t) {
    return t.getHead();
  }
};

template<unsigned N, typename... Types>
auto get(Tuple<Types...> const& t) {
  return TupleGet<N>::apply(t);
}
```

请注意，函数模板 `get` 只是对 `TupleGet` 的静态成员函数进行调用的一层薄的包装。这一技巧实际上是函数模板缺乏偏特化的一种变通方案，我们用该函数模板对 N 的值进行特化。在递归分支（N > 0），静态成员函数 `apply()` 提取当前元组的尾部，并递减 N 以继续在元组后面寻找要找的元素。基础分支（N = 0）返回当前元组的头部，完成实现。

### 构造

为了使元组更有用，我们需要既能从一组独立的值（每个元素一个）中构造它，也能从另一个元组中构造它。从一组独立的值中拷贝构造元组，传递第 1 个值以初始化头部元素（通过其基类），然后传递其余的值给表示尾部的基类。

```c++
Tuple<Head const& head, Tail const&... tail>
  : head(head), tail(tail...) {}
```

这就实现了本章开头介绍的 `Tuple` 的例子。

```c++
Tuple<int, double, std::string> t(17, 3.14, "Hello, World!");
```

然而，这并不是最为通用的接口。用户可能希望用移动构造来初始化某些（也许不是全部）元素，或让一个元素从不同类型的值中构造。因此，我们应该使用完美转发来初始化元组：

```c++
template<typename VHead, typename... VTail>
Tuple(VHead&& vhead, VTail&&... vtail)
  : head(std::forward<VHead>(vhead)),
    tail(std::forward<VTail>(vtail)...) {}
```

接着，实现从另一个元组中构造新元组：

```c++
template<typename VHead, typename... VTail>
Tuple(Tuple<VHead, VTail...> const& other)
  : head(other.getHead()), tail(other.getTail()) {}
```

然而，该构造函数的引入并不足以支持元组的转换。给定前文的元组 `t`，试图以兼容类型构造另一个元组会失败：

```c++
// 错误：不存在由 Tuple<int, double, string> 到 long 的转换
Tuple<long int, long double, std::string> t2(t);
```

这里的问题是，试图以一组独立值进行初始化的构造函数模板比接收元组的构造函数模板更匹配。为解决这个问题，必须使用 `std::enable_if<>`。当尾部没有预期的长度时，关闭两个成员函数模板：

```c++
template<typename VHead, typename... VTail,
         typename = std::enable_if_t<sizeof...(VTail)==sizeof...(Tail)>>
Tuple(VHead&& vhead, VTail&&... vtail)
  : head(std::forward<VHead>(vhead)),
    tail(std::forward<VTail>(vtail)...) {}

template<typename VHead, typename... VTail,
         typename = std::enable_if_t<sizeof...(VTail)==sizeof...(Tail)>>
Tuple(Tuple<VHead, VTail...> const& other)
  : head(other.getHead()), tail(other.getTail()) {}
```

完整的构造函数声明在 tuples/tuple.hpp 中。

`makeTuple()` 函数模板使用类型推导来确定它所返回的 `Tuple` 的元素类型，这大大方便了从一个给定的元素集合中构造元组。

```c++
template<typename... Types>
auto makeTuple(Types&&... elems) {
  return Tuple<std::decay_t<Types>...>(std::forward<Types>(elems)...);
}
```

同样，我们使用完美转发并结合 `std::decay<>` 特征，将字符串字面量和其他原始数组转换为指针，并删除 `const` 和引用。

## 基础元组运算

### 比较

元组是包含其他值的结构类型。要比较两个元组，只需比较它们的元素。因此，我们可以写一个 `operator==` 运算符的定义来按元素比较两个元组：

```c++
// 基础分支
bool operator==(Tuple<> const&, Tuple<> const&) {
  // 空元组恒等
  return true;
}

// 递归分支
template<typename Head1, typename... Tail1,
         typename Head2, typename... Tail2,
         typename = std::enable_if_t<sizeof...(Tail1)==sizeof...(Tail2)>>
bool operator==(Tuple<Head1, Tail1...> const& lhs,
                Tuple<Head2, Tail2...> const& rhs) {
  return lhs.getHead() == rhs.getHead() &&
         lhs.getTail() == rhs.getTail();
}
```

如同许多类型列表和元组的算法，按元素比较先访问头部元素，然后递归访问尾部元素，最终会遇到基础分支。`!=`、`<`、`>`、`<=` 和 `>=` 运算符的定义也类似。

### 输出

下面的 `operator<<` 运算符输出任意元组，只要其元素类型可被输出。

```c++
#include <iostream>

void printTuple(std::ostream& strm, Tuple<> const&, bool isFirst = true)
{
  strm << ( isFirst ? '(' : ')' );
}

template<typename Head, typename... Tail>
void printTuple(std::ostream& strm, Tuple<Head, Tail...> const& t,
                bool isFirst = true) {
  strm << ( isFirst ? "(" : ", " );
  strm << t.getHead();
  printTuple(strm, t.getTail(), false);
}

template<typename... Types>
std::ostream& operator<<(std::ostream& strm, Tuple<Types...> const& t) {
  printTuple(strm, t);
  return strm;
}
```

现在，创建和显示元组就比较容易了。

## 元组算法

元组是一种容器，它提供的功能有访问和修改每个元素（通过 `get()`）、创建新元组（直接地或通过 `makeTuple()`）和将元组分解成头部和尾部（通过 `getHead()` 和 `getTail()`）。这些基本功能足以用来打造成套的元组算法，如为元组增加或删除元素、重新排列元素，或选择元组内的元素的某个子集。

元组算法特别有趣，因为它们既需要编译期计算，也需要运行期计算。就像第 24 章的类型列表算法一样，对元组应用算法可能会产生类型完全不同的元组，这需要编译期计算。例如，将 `Tuple<int, double, string>` 反转会产生 `Tuple<string, double, int>`。然而，就像同质容器的算法（例如，对 `std::vector` 应用 `std::reverse()`），元组算法实际上需要在运行期执行代码，我们要对所生成代码的效率心中有数。

### 作为类型列表

如果忽略我们的 `Tuple` 模板的实际运行期组件，我们会发现它的结构与第 24 章中开发的 `Typelist` 模板完全一样：它接收任意数量的模板类型形参。事实上，通过一些偏特化，我们可以把 `Tuple` 变成具有全功能的类型列表。

```c++
// 判断元组是否为空
template<>
struct IsEmpty<Tuple<>> {
  static constexpr bool value = true;
};

// 提取前面的元素
template<typename Head, typename... Tail>
class FrontT<Tuple<Head, Tail...>> {
 public:
  using Type = Head;
};

// 删除前面的元素
template<typename Head, typename... Tail>
class PopFrontT<Tuple<Head, Tail...>> {
 public:
  using Type = Tuple<Tail...>;
};

// 将元素加到前面
template<typename... Types, typename Element>
class PushFrontT<Tuple<Types...>, Element> {
 public:
  using Type = Tuple<Element, Types...>;
};

// 将元素加到后面
template<typename... Types, typename Element>
class PushBackT<Tuple<Types...>, Element> {
 public:
  using Type = Tuple<Types..., Element>;
};
```

现在，第 24 章中开发的所有类型列表算法对 `Tuple` 和 `Typelist` 同样有效，于是我们就可以轻松处理元组的类型。

下面马上会看到，应用于元组的类型列表算法常被用来帮助确定元组算法的结果类型。

### 增删

能在元组的头部和尾部增加一个元素是重要的功能，有了它我们就可以构建更高级的算法来操纵元组中的值。与类型列表一样，再元组的前面插入元素要比在后面插入元素容易得多，所以我们从 `pushFront()` 开始。

```c++
template<typename... Types, typename V>
PushFront<Tuple<Types...>, V>
pushFront(Tuple<Types...> const& tuple, V const& value) {
  return PushFront<Tuple<Types...>, V>(value, tuple);
}
```

在现有元组的前面添加一个新的元素（称为 `value`），需要我们形成一个以 `value` 为头部，以现有元组为尾部的新元组。生成的元组类型是 `Tuple<V, Types...>`。然而，我们选择使用类型列表算法 `PushFront()` 来演示元组算法的编译期层面与运行期层面的紧密耦合，编译期的 `PushFront()` 计算出需要构造的类型，运行期用它来生成合适的值。

在现有元组的末尾添加一个新的元素就比较复杂了，因为这需要对元组进行递归遍历，边遍历边构建修改后的元组。请注意 `pushBack()` 实现的结构，看它如何遵循 24.2.3 节中类型列表 `PushBack()` 的递归实现方式。

```c++
// 基础分支
template<typename V>
Tuple<V> pushBack(Tuple<> const&, V const& value)
{
  return Tuple<V>(value);
}

// 递归分支
template<typename Head, typename... Tail, typename V>
Tuple<Head, Tail..., V>
pushBack(Tuple<Head, Tail...> const& tuple, V const& value) {
  return Tuple<Head, Tail..., V>(tuple.getHead(),
                                 pushBack(tuple.getTail(), value));
}
```

不出所料，基础分支将一个值追加到一个长度为 0 的元组，它产生一个只包含该值的元组。在递归分支中，我们将列表起始处的当前元素（`tuple.getHead()`）和添加新元素到列表尾部的结果元组（由递归的 `pushBack()` 调用得到）组装起来，形成一个新的元组。尽管我们选择将构造的类型表达为 `Tuple<Head, Tail..., V>`，请注意，这相当于使用编译期的 `PushBack<Tuple<Head, Tail...>, V>`。

同样容易实现 `popFront()`：

```c++
template<typename... Types>
PopFront<Tuple<Types...>> popFront(Tuple<Types...> const& tuple) {
  return tuple.getTail();
}
```

### 反转

元组的元素仍然可以用递归元组算法来反转，其结构遵循 24.2.4 节的类型列表反转的递归实现方式：

```c++
// 基础分支
Tuple<> reverse(Tuple<> const& t) {
  return t;
}

// 递归分支
template<typename Head, typename... Tail>
Reverse<Tuple<Head, Tail...>> reverse(Tuple<Head, Tail...> const& t) {
  return pushBack(reverse(t.getTail()), t.getHead());
}
```

基础分支不必细说，而递归分支则是将列表的尾部反转，并把当前的头部追加其后。

与类型列表类似，现在可以对临时反转的列表调用 `popFront()`，结合使用 24.2.4 节的 `PopBack()`，从而轻松提供 `popBack()`：

```c++
template<typename... Types>
PopBack<Tuple<Types...>> popBack(Tuple<Types...> const& tuple) {
  return reverse(popFront(reverse(tuple)));
}
```

### 索引列表

25.3.3 节中元组反转的递归写法是正确的，但在运行期，它的低效令它显得毫无用处。

太多次拷贝了！在元组反转的理想实现中，每个元素只会被拷贝一次，即从源元组直接拷贝到结果元组中的正确位置。通过小心地使用引用，包括使用对中间实参类型的引用，我们可以实现这个目标。但是这样做会让实现变得相当复杂。

为了消除元组反转中不必要的拷贝，可以简单地使用 `makeTuple()` 和 `get()` 做到这一点：

```c++
auto reversed = makeTuple(get<4>(copies), get<3>(copies),
                          get<2>(copies), get<1>(copies),
                          get<0>(copies));
```

索引列表（也称为索引序列，参阅 24.4 节）泛化了这一观念，其将元组索引的集合捕获到一个形参包中，这样就能通过包扩展来产生 `get` 调用序列。这就让索引运算和索引列表的实际应用可以分离开来，索引运算可以是任意复杂的模板元程序，而索引列表的实际应用则重在运行期效率。标准类型 `std::integer_sequence`（在 C++14 中引入）常被用来表示索引列表。

### 用索引列表反转

为了用索引列表进行元组反转，我们首先需要一个索引列表的表示。索引列表是一种包含数值的类型列表，目的是作为类型列表或异质数据结构的索引使用（参阅 24.4 节）。我们使用 24.3 节中开发的 `Valuelist` 类型表示我们的索引列表。与上面的元组反转例子相对应的索引列表是：

```c++
Valuelist<unsigned, 4, 3, 2, 1, 0>
```

如何产生这样的索引列表呢？一种方式是，首先生成一个索引列表，从 0 到 N-1（包含）递增计数，其中 N 是一个元组的长度。用一个简单的模板元程序 `MakeIndexList` 来实现：

```c++
// 递归分支
template<unsigned N, typename Result = Valuelist<unsigned>>
struct MakeIndexListT
  : MakeIndexListT<N-1, PushFront<Result, CTValue<unsigned, N-1>>> {};

// 基础分支
template<typename Result>
struct MakeIndexListT<0, Result> {
  using Type = Result;
};

template<unsigned N>
using MakeIndexList = typename MakeIndexListT<N>::Type;
```

然后我们便能将此运算与类型列表的 `Reverse` 组合来产生合适的索引列表：

```c++
using MyIndexList = Reverse<MakeIndexList<5>>;
// equivalent to Valuelist<unsigned, 4, 3, 2, 1, 0>
```

为了实际执行反转，需要将索引列表中的索引捕获到非类型形参包中。这通过将索引集合元组的 `reverse()` 算法分为两部分来处理：

```c++
template<typename... Elements, unsigned... Indices>
auto reverseImpl(Tuple<Elements...> const& t,
                 Valuelist<unsigned, Indices...>) {
  return makeTuple(get<Indices>(t)...);
}

template<typename... Elements>
auto reverse(Tuple<Elements...> const& t) {
 return reverseImpl(t, Reverse<MakeIndexList<sizeof...(Elements)>>());
}
```

在 C++11 中，返回类型必须声明为

```c++
-> decltype(makeTuple(get<Indices>(t)...))
```

以及

```c++
-> decltype(reverseImpl(t, Reverse<MakeIndexList<sizeof...(Elements)>>()))
```

`reverseImpl()` 函数模板从它的 `Valuelist` 参数中将索引捕获到形参包 `Indices` 中。然后用捕获的索引形成的索引集合对元组调用 `get()` 形成实参，以此实参调用 `makeTuple()` 得到返回结果。

正如前面所讨论的，`reverse` 算法本身只形成了合适的索引集合，并将其提供给 `reverseImpl` 算法。索引是作为模板元程序被操作的，因此它不产生运行期代码。仅有的运行期代码在 `reverseImpl` 中，它使用 `makeTuple()` 一步构建结果元组，因此只对元组元素进行一次拷贝。

### 重排和选择

25.3.5 节中用来形成反转元组的 `reverseImpl()` 函数模板实际上不包含 `reverse()` 运算的具体代码。更准确地说，它只从某个现有的元组中选择一组特定的索引，并使用它们来形成一个新的元组。`reverse()` 提供了一个反转的索引集合，而许多算法都可以建立在这个核心的元组 `select()` 算法之上。

```c++
template<typename... Elements, unsigned... Indices>
auto select(Tuple<Elements...> const& t,
            Valuelist<unsigned, Indices...>) {
  return makeTuple(get<Indices>(t)...);
}
```

元组的“溅”（splat）运算就是建立在 `select()` 之上的一个简单算法，它从元组中抽取并拷贝某个元素来创建另一个元组，其中包含该元素一定数量的副本。比如：

```c++
Tuple<int, double, std::string> t1(42, 7.7, "hello");
auto a = splat<1, 4>(t);
std::cout << a << '\n';
```

将产生 `Tuple<double, double, double, double>`，它的每一个值都是一份 `get<1>(t)` 的副本，所以会输出

```c++
(7.7, 7.7, 7.7, 7.7)
```

`splat()` 是 `select()` 的一个直接应用，通过给定一个元程序，产生一个“要被拷贝的”索引集合，包含值 `I` 的 N 个副本。

```c++
template<unsigned I, unsigned N, typename IndexList = Valuelist<unsigned>>
class ReplicatedIndexListT;
 
template<unsigned I, unsigned N, unsigned... Indices>
class ReplicatedIndexListT<I, N, Valuelist<unsigned, Indices...>>  
  : public ReplicatedIndexListT<I, N-1,
                                Valuelist<unsigned, Indices..., I>> {};

template<unsigned I, unsigned... Indices>
class ReplicatedIndexListT<I, 0, Valuelist<unsigned, Indices...>> {
 public:
  using Type = Valuelist<unsigned, Indices...>;
};

template<unsigned I, unsigned N>
using ReplicatedIndexList = typename ReplicatedIndexListT<I, N>::Type;

template<unsigned I, unsigned N, typename... Elements>
auto splat(Tuple<Elements...> const& t) {
  return select(t, ReplicatedIndexList<I, N>());
}
```

通过模板元程序来操纵索引列表，在对其应用 `select()`，即便是复杂的元组算法也能实现。例如，我们可以使用 24.2.7 节中开发的插入排序，来按元素类型的大小对元组中的元素进行排序。给出这样一个 `sort()` 函数，它接收一个比较元组元素类型大小的模板元函数进行比较运算，我们就可以用如下代码按类型大小对元组元素进行排序：

```c++
#include <complex>

template<typename T, typename U>
class SmallerThanT {
 public:
  static constexpr bool value = sizeof(T) < sizeof(U);
};

void testTupleSort() {
  auto T1 = makeTuple(17LL, std::complex<double>(42, 77), 'c', 42, 7.7);
  std::cout << t1 << '\n';
  auto T2 = sort<SmallerThanT>(t1); // t2 is Tuple<int, long, std::string>
  std::cout << "sorted by size: " << t2 << '\n';
}
```

输出可能如下：

```c++
(17, (42,77), c, 42, 7.7)
sorted by size: (c, 42, 7.7, 17, (42,77))
```

实际的 `sort()` 实现涉及以元组的 `select()` 来使用 `InsertionSort`。

```c++
// 比较元组中元素的元函数的包装
template<typename List, template<typename T, typename U> class F>
class MetafunOfNthElementT {
 public:
  template<typename T, typename U> class Apply;

  template<unsigned N, unsigned M>
  class Apply<CTValue<unsigned, M>, CTValue<unsigned, N>>
    : public F<NthElement<List, M>, NthElement<List, N>> {};
};

// 基于元素类型大小的比较对元组元素排序
template<template<typename T, typename U> class Compare,
         typename... Elements>
auto sort(Tuple<Elements...> const& t) {
  return select(t,
                InsertionSort<MakeIndexList<sizeof...(Elements)>,
                              MetafunOfNthElementT<
                                         Tuple<Elements...>,
                                         Compare>::template Apply>());
}
```

请仔细观察 `InsertionSort` 的使用：实际上要被排序的类型列表是由指向类型列表元素的索引组成的列表，它由 `MakeIndexList<>` 构造。因此，插入排序的结果也是由指向元组元素的索引组成的集合，它随后被提供给 `select()`。然而，因为 `InsertionSort` 是对索引进行操作，所以它期望它的比较运算能够比较两个索引。考虑非元编程的例子，比如对 `std::vector` 的索引进行排序，就更容易理解其原理：

```c++
#include <vector>
#include <algorithm>
#include <string>

int main() {
  std::vector<std::string> strings = {"banana", "apple", "cherry"};
  std::vector<unsigned> indices = { 0, 1, 2 };
  std::sort(indices.begin(), indices.end(),
            [&strings](unsigned i, unsigned j) {
                return strings[i] < strings[j];
            });
}
```

这里，`indices` 包含指向 `vector strings` 中元素的索引。`sort()` 运算对实际索引进行排序，所以要提供 lambda 进行比较运算，它接收两个 `unsigned` 值（而不是 `string` 值）。而 lambda 的函数体使用这些 `unsigned` 值作为指向 `strings` 这一 `vector` 中元素的索引，所以排序实际上是根据 `strings` 的内容进行的。排序结束时，`indices` 提供指向 `strings` 中元素的索引，并且它们已经根据元素值排好序了。

我们在元组的 `sort()` 中使用 `InsertionSort` 也采用了同样的方法。适配器模板 `MetafunOfNthElementT` 提供了一个模板元函数（其嵌套的 `Apply`），它接收两个索引（`CTValue` 的特化），并使用 `NthElement` 从其 `Typelist` 实参中提取相应的元素。在某种意义上，成员模板 `Apply` 已“捕获”了提供给它所在模板（`MetafunOfNthElementT`）的类型列表，就像 lambda 从它所在的作用域捕获 `strings` 这一 `vector` 一样。然后 `Apply` 将提取的元素类型转发给底层元函数 `F`，完成适配。

请注意，本节的所有排序计算都在编译期进行，并直接形成结果元组，在运行期没有额外的值拷贝。

## 展开元组

元组可用于将一组相关的值存储在一起，成为一个单一的值，无论这些相关的值有什么类型或有多少。有时，可能需要对这样的元组进行展开，例如，将其元素作为单独的参数传递给函数。

我们可以使用索引列表实现这一目的。下面的函数模板 `apply()` 接收一个函数和一个元组，它以展开的元组元素调用函数：

```c++
template <typename F, typename... Elements, unsigned... Indices>
auto applyImpl(F f, Tuple<Elements...> const& t,
                    Valuelist<unsigned, Indices...>)
  ->decltype(f(get<Indices>(t)...)) {
  return f(get<Indices>(t)...);
}

template <typename F, typename... Elements,
          unsigned N = sizeof...(Elements)>
auto apply(F f, Tuple<Elements...> const& t)
  ->decltype(applyImpl(f, t, MakeIndexList<N>())) {
  return applyImpl(f, t, MakeIndexList<N>());
}
```

`applyImpl()` 函数模板接收一个给定的索引列表并用它来展开元组的元素，将其放入它的函数对象形参 `f` 的形参列表中。面向使用者的 `apply()` 只负责构造初始的索引列表。

C++17 提供了一个类似的函数，适用于任意类似于元组的类型。

## 优化元组

元组是一种基本的异质容器，有大量的潜在用途。因此，值得考虑如何优化它在运行期（如减少存储、执行时间）和编译期（如减少模板实例化的数目）的使用。本节将讨论对上文中的 `Tuple` 实现的一些具体优化。

### 元组和 EBCO

对于 `Tuple` 的存储方式，我们使用了比严格意义上所需更多的存储空间。一个问题是，`tail` 成员终究是一个空元组，因为每个非空元组都以一个空元组结束，而数据成员至少占用一个字节的存储空间。

为提高 `Tuple` 的存储效率，我们可以应用空基类优化（EBCO），让元组从尾部元组继承，而不是让尾部元组成为成员。例如：

```c++
template<typename Head, typename... Tail>
class Tuple<Head, Tail...> : private Tuple<Tail...> {
 private:
  Head head;
 public:
  Head& getHead() { return head; }
  Head const& getHead() const { return head; }
  Tuple<Tail...>& getTail() { return *this; }
  Tuple<Tail...> const& getTail() const { return *this; }
};
```

这与 21.1.2 节中对 `BaseMemberPair` 采取的方法相同。遗憾的是，实际上它产生副作用，即颠倒构造函数中元组元素初始化的顺序。先前，因为 `head` 成员在 `tail` 成员之前，所以 `head` 会先被初始化。在新的 `Tuple` 存储方式里，尾部在基类中，所以它将在成员 `head` 之前被初始化。

为解决这一问题，可以将 `head` 成员放入元组的基类，把它放在基类列表中的尾部基类之前。该方法的直接实现将引入一个 `TupleElt` 模板，用于包装每个元素类型，于是 `Tuple` 可以从它继承：

```c++
template<typename... Types>
class Tuple;

template<typename T>
class TupleElt {
  T value;

 public:
  TupleElt() = default;

  template<typename U>
  TupleElt(U&& other) : value(std::forward<U>(other)) {}

  T&       get()       { return value; }
  T const& get() const { return value; }
};

 // 递归分支
template<typename Head, typename... Tail>
class Tuple<Head, Tail...>
  : private TupleElt<Head>, private Tuple<Tail...> {
 public:
  Head& getHead() {
    // 潜在歧义
    return static_cast<TupleElt<Head>*>(this)->get();
  }

  Head const& getHead() const {
    // 潜在歧义
    return static_cast<TupleElt<Head> const*>(this)->get();
  }
  Tuple<Tail...>& getTail() { return *this; }
  Tuple<Tail...> const& getTail() const { return *this; }
};

// 基础分支
template<>
class Tuple<> {
// 无需存储
};
```

虽然这种方法解决了初始化先后的问题，但它引入了一个新的（更糟糕的）问题：我们无法再从一个有两个相同类型元素的元组中提取元素，比如 `Tuple<int, int>`，因为从元组到该相同类型的 `TupleElt`（比如 `TupleElt<int>`）进行的派生类到基类的转换将是有歧义的。

为了消除这种歧义，我们需要确保每个 `TupleElt` 基类在一个给定的 `Tuple` 中是唯一的。一种方法是在其元组内对元素的“高度”进行编码，也就是对尾部元组的长度进行编码。元组中的最后一个元素将被存储为高度 0，倒数第 2 个元素将被存储为高度 1，以此类推。

```c++
template<unsigned Height, typename T>
class TupleElt {
  T value;
 public:
  TupleElt() = default;

  template<typename U>
  TupleElt(U&& other) : value(std::forward<U>(other)) {}

  T&       get() { return value; }
  T const& get() const { return value; }
};
```

有了这个解决方案，我们可以产生一个应用了 ECBO 的 `Tuple`，同时保持初始化次序合理和支持同一类型的多个元素

```c++
template<typename... Types>
class Tuple;

// 递归分支
template<typename Head, typename... Tail>
class Tuple<Head, Tail...>
  : private TupleElt<sizeof...(Tail), Head>, private Tuple<Tail...> {
  using HeadElt = TupleElt<sizeof...(Tail), Head>;
 public:
  Head& getHead() {
    return static_cast<HeadElt*>(this)->get();
  }
  Head const& getHead() const {
    return static_cast<HeadElt const*>(this)->get();
  }
  Tuple<Tail...>& getTail() { return *this; }
  Tuple<Tail...> const& getTail() const { return *this; }
};

// 基础分支
template<>
class Tuple<> {
  // 无需存储
};
```

有了这个实现，下面的程序：

```c++
#include <algorithm>
#include "tupleelt1.hpp"
#include "tuplestorage3.hpp"
#include <iostream>

struct A {
  A() {
    std::cout << "A()" << '\n';
  }
};

struct B {
  B() {
    std::cout << "B()" << '\n';
  }
};

int main() {
  Tuple<A, char, A, char, B> t1;
  std::cout << sizeof(t1) << " bytes" << '\n';
}
```

输出

```
A()
A()
B()
5 bytes
```

ECBO 已经消除了一个字节（对于空元组 `Tuple<>`）。请注意，`A` 和 `B` 都是空类，这提示在 `Tuple` 中还有一次应用 ECBO 的机会。`TupleElt` 可以稍微进行扩展，在安全的情况下继承元素类型，而不需要改变 `Tuple`。

```c++
#include <type_traits>

template<unsigned Height, typename T,
         bool = std::is_class<T>::value && !std::is_final<T>::value>
class TupleElt; 

template<unsigned Height, typename T>
class TupleElt<Height, T, false> {
  T value;

 public:
  TupleElt() = default;
  template<typename U>
  TupleElt(U&& other) : value(std::forward<U>(other)) {} 
  T&       get() { return value; }
  T const& get() const { return value; }
};

template<unsigned Height, typename T>
class TupleElt<Height, T, true> : private T {
 public:
  TupleElt() = default;
  template<typename U>
  TupleElt(U&& other) : T(std::forward<U>(other)) {}
 
  T&       get() { return *this; }
  T const& get() const { return *this; }
};
```

当提供给 `TupleElt` 的是一个非 `final` 的类的时候，它会私有地继承该类，以允许把 EBCO 应用到存储的值。有了这个改动，前面的程序现在输出

```
A()
A()
B()
2 bytes
```

### 常数时间复杂度的 `get()`

在使用元组时，`get()` 操作极为常见，但是它的递归实现需要线性数量的模板实例化，这会影响编译时间。幸好，25.5.1 节介绍的 EBCO 也可以让 `get()` 的实现更加高效，下面来具体介绍。

关键在于，模板实参推导在将一个形参（基类类型）与一个实参（派生类类型）相匹配时，推导出基类的模板实参。因此，如果可以计算出希望提取的元素的高度 `H`，我们就可以依赖从 `Tuple` 特化到 `TupleElt<H, T>`（其中 `T` 是推导出来的）的转换来提取该元素，而无需手动遍历所有的索引。

```c++
template <unsigned H, typename T>
T& getHeight(TupleElt<H,T>& te) {
  return te.get();
}

template<typename... Types>
class Tuple; 

template<unsigned I, typename... Elements>
auto get(Tuple<Elements...>& t) -> decltype(getHeight<sizeof...(Elements) - I - 1>(t)) {
  return getHeight<sizeof...(Elements) - I - 1>(t);
}
```

因为 `get<I>(t)` 接收的是所需元素的索引 `I`（从元组的开始算起），而元组实际上是按照高度 `H` 存储的（从元组的结束算起），所以我们从 `I` 中计算出 `H`。为 `getHeight()` 调用而进行的模板实参推导执行实际的搜索：高度 `H` 是固定的，因为它是在调用中显式提供的，所以只有一个 `TupleElt` 基类将被匹配，从它那里将推导出类型 `T`。请注意，`getHeight()` 必须被声明为 `Tuple` 的友元，以允许到私有基类的转换。例如：

```c++
// 在类模板 Tuple 的递归分支内部
template <unsigned I, typename... Elements>
friend auto get(Tuple<Elements...>& t)
  -> decltype(getHeight<sizeof...(Elements) - I - 1>(t));
```

请注意，这个实现只需要常数数量的模板实例化，因为我们已经将匹配索引的艰苦工作转移给了编译器的模板参数推导引擎。
