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
