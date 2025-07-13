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
