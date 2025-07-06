# 模板与继承

## 空基类优化

C++ 的类常是“空”的，空的意思是他们的内部表示在运行期不需要占用任何内存。这对于仅包含类型成员、非虚函数成员和静态数据成员的类是典型的情况。非静态数据成员、虚函数和虚基类则是另一种情况，它们确实需要在运行期占用内存。

不过，即使是空类，其大小也是非零的。在许多平台中，其大小是 1 字节。有个别系统对于类类型强制要求更严格的对齐，可能是其他（通常是 4）字节。

### 布局原则

C++ 的设计者有种种理由来避免大小为 0 的类。例如，以大小为 0 的类为元素的数组的大小也将为 0，但这样指针运算的通常特性就不再适用了。

然而，尽管 C++ 中没有大小为 0 的类型，C++ 标准的确规定当以空类作为基类时，不需要为其分配空间，前提是这样做不会导致它被分配到与其他对象或者同类型的子对象相同的地址上。让我们以示例来阐明，在实践中空基类优化（empty base class optimization，EBCO）意味着什么。

```c++
class Empty {
    using Int = int;    // 类型别名成员不会让一个类成为非空类
};

class EmptyToo : public Empty {};

class EmptyThree : public EmptyToo {};
```

如果你用的编译器实现了 EBCO，它会为每个类输出相同的大小，但是没有类的大小为 0。这意味着在 `EmptyToo` 类中的 `Empty` 类不会被分配任何空间。同时请注意，一个继承自被优化的空基类（而没有其他基类）的空类仍然是空的。这解释了为什么 `EmptyThree` 类也能有和 `Empty` 类同样的大小。如果你用的编译器没有实现 EBCO，程序会输出不同的大小。

```c++
class Empty {
    using Int = int;    // 类型别名成员不会让一个类成为非空类
};

class EmptyToo : public Empty {};

class NonEmpty : public Empty, public EmptyToo {};
```

`NonEmpty` 类并不是空类，这可能有点让人吃惊。毕竟，它没有任何成员，它的基类也没有。然而，`NonEmpty` 的基类 `Empty` 和 `EmptyToo` 不能分配到同一个地址，因为这将导致 `EmptyToo` 的基类 `Empty` 最终与 `NonEmpty` 的基类 `Empty` 位于同一个地址。换句话说，两个相同类型的子对象最终会在同一个偏移量上，而这是 C++ 的对象布局规则所不允许的。

对 EBCO 进行限制的理由是期望能比较两个指针是否指向同一个对象。因为指针在程序内部几乎总是仅表示为地址，我们必须确保两个不同的地址（即指针值）对应两个不同的对象。

这个限制也许看起来不是非常重要。然而，在实践中经常会遇到相关问题，因为许多类往往继承自某些空类的一个小集合，而这些空类又往往定义了一些共同的类型别名。当这样的类的两个子对象被用在同一个完整对象中时，优化就会被阻止。

就算有此限制，EBCO 仍是模板库的一个重要优化，因为有些技巧要依赖于某些基类的引入，而引入这些基类只是为了引入新的类型别名或者在不增加新数据的情况下提供额外功能。本章会描述几个这样的技巧。

### 作为基类的成员

对于数据成员，不存在类似 EBCO 的技术，因为这会制造出指向成员的指针的表示方面（除了其他方面以外）的问题。那么我们不妨考虑将成员变量实现为（私有）基类的形式，而且第一眼看来，该类型确实也可以成为成员变量的类型，不过这都需要我们在后面对该类型进行特殊处理。

这一问题在模板的语境中最有意思，因为模板参数经常会被空的类类型替换，但一般我们无法依赖这一规律。如果对于模板类型参数一无所知，就无法轻易利用 EBCO。

```c++
template <typename T1, typename T2>
class MyClass {
  private:
    T1 a;
    T2 b;
    ...
};
```

完全可能有一个或者两个模板参数被空的类类型替换。如果确实如此，那么 `MyClass<T1, T2>` 这种表示可能是次优的，每一个实例都可能浪费一个字的内存。

采用让模板参数成为基类的方案可以避免这种浪费：

```c++
template <typename T1, typename T2>
class MyClass : private T1, private T2 {};
```

然而，这种直截了当的替代方案也自有其问题，如下所示。

- 当 `T1` 或 `T2` 被一个非类的类型或者被联合体类型替换时，它就不起作用了。
- 当两个参数都被同一个类型替换时，它也不起作用了（尽管通过增加另外一层的继承可以相对容易地解决该问题）。
- 模板参数的类可能被标记为 `final`，继承它会导致错误。

就算圆满地解决了这些问题，还有一个非常严重的问题一直存在：加入基类，会从根本上改动该类的接口。对于 `MyClass`，该问题看起来并不严重，因为只会影响到极少的接口元素，但是正如在本章后面会展示的那样，从一个模板形参继承会影响到一个成员函数是不是虚函数。显而易见，这种利用 EBCO 的方式充满了麻烦。

可以针对一种常见情况设计更为现实的手段，即当仅有一个模板形参会被替换为类类型。而类模板还有另一个可用的成员时。主要的想法是使用 EBCO 将潜在的空类型参数与另一个成员“合并”。

```c++
template <typename CustomClass>
class Optimizable {
  private:
    CustomClass info;   // 可能为空
    void*       storage;
    ...
};
```

模板实现者可以使用如下写法：

```c++
template <typename CustomClass>
class Optimizable {
  private:
    BaseMemberPair<CustomClass, void*> info_and_storage;
    ...
};
```

即使不看模板类 `BaseMemberPair` 的实现，显然使用它也让 `Optimizable` 的实现更“啰嗦”。然而，据多个模板库实现者的报告，其在性能上的收益（对于他们的客户来说）值得为之付出额外的复杂度。

`BaseMemberPair` 的实现可以做到相当紧凑：

```c++
template <typename Base, typename Member>
class BaseMemberPair : private Base {
  private:
    Member mem;
  public:
    // 构造函数
    BaseMemberPair(Base const& b, Member const& m)
      : Base(b), mem(m) {}
    
    // 通过 base() 访问基类的数据
    Base const& base() const {
        return static_cast<Base const&>(*this);
    }

    Base& base() {
        return static_cast<Base&>(*this);
    }

    // 通过 member() 访问成员的数据
    Member const& member() const {
        return this->mem;
    }

    Member& member() {
        return this->mem;
    }
};
```

类的实现中，需要使用成员函数 `base()` 和 `member()` 来访问被封装（并且可能做了存储优化）的数据成员。

## 奇妙递归模板模式

另一个模式是奇妙递归模板模式（curiously recurring template pattern，CRTP）。这个命名古怪的模式指的是一类技巧，要点在于将派生类作为模板实参传给它自己的某个基类。该模式的最简单的 C++ 代码实现如下：

```c++
template <typename Derived>
class CuriousBase {
    ...
};

class Curious : public CuriousBase<Curious> {
    ...
};
```

这段 CRTP 实现代码展示了一个非依赖型基类：`Curious` 类不是模板，因此免于与依赖性基类的名字可见性问题纠缠。然而，这不是 CRTP 的本质特征。确实，我们同样可以使用下面所示的另一种方式：

```c++
template <typename Derived>
class CuriousBase {
    ...
};

template <typename T>
class CuriousTemplate : public CuriousBase<CuriousTemplate<T>> {
    ...
};
```

以模板形参将派生类传给它的基类，这样基类不必使用虚函数就可以对派生类定制自己的行为。这样就利用 CRTP 避免了那些只能使用成员函数（例如构造函数、析构函数以及索引运算符）的实现，或是依赖于派生类标识的实现。

CRTP 的一种简单应用是记录某个类类型的对象被创造的总个数。这可以通过引入一个静态数据成员并在构造函数和析构函数中对其进行增减实现。然而，在每个类中都提供这部分代码就“啰嗦”了，而通过单个（非 CRTP）基类去实现这一功能则会把不同的派生类的个数混在一起。使用更好的方式，我们可以写出如下的模板：

```c++
template <typename CountedType>
class ObjectCounter {
  private:
    inline static std::size_t count = 0;    // 现存对象的个数

  protected:
    // 默认构造函数
    ObjectCounter() {
        ++count;
    }

    // 拷贝构造函数
    ObjectCounter(ObjectCounter<CountedType> const&) {
        ++count;
    }

    // 移动构造函数
    ObjectCounter(ObjectCounter<CountedType>&&) {
        ++count;
    }

    // 析构函数
    ~ObjectCounter() {
        --count;
    }

public:
    // 返回现有成员的个数
    static std::size_t live() {
        return count;
    }
};
```

请注意，这里使用 `inline` 来定义和初始化类结构中的 `count` 成员。在 C++17 之前，只能在类模板的外面定义它：

```c++
template <typename CountedType>
class ObjectCounter {
  private:
    static std::size_t count;   // 现存对象的个数
    ...
};

// 以 0 初始化计数器
template <typename CountedType>
std::size_t ObjectCounter<CountedType>::count = 0;
```

如果我们想清点某个类现有（也就是说还没被销毁）的对象，从 `ObjectCounter` 模板派生出这个类就够了。

### Barton-Nackman 技巧

在现代 C++ 中，同索性定义一个普通函数模板的做法相比，在类模板中定义友元函数的唯一优势在于语法：友元函数声明可以访问它们所在类的 `private` 和 `protected` 成员，而不需要再写出所在类模板的所有模板形参。然而，友元函数定义和 CRTP 组合使用的时候另有一番用处。

### 运算符实现

当实现一个提供运算符重载的类的时候，通常的做法是提供一些不同（但是相关）的运算符的重载。例如，实现了相等（`==`）运算符的类往往也会实现不等（`!=`）运算符，而实现了小于（`<`）运算符的类往往也会实现其他关系运算符（如 `>`、`<=`、`>=`）。在许多类中，这些运算符中往往只有一个是确实值得注意的，而其他都可以通过它定义出来。例如，类 `X` 的 `!=` 运算符往往是由 `==` 运算符定义出来的：

```c++
bool operator!= (X const& x1, X const& x2) {
    return !(x1 == x2);
}
```

由于大量的类都像这样来定义 `!=`，因此很容易想到把它推广成模板：

```c++
template <typename T>
bool operator!= (T const& x1, T const& x2) {
    return !(x1 == x2);
}
```

实际上，C++ 标准库的 `<utility>` 头文件就包含类似的定义。然而，这些定义（对于 `!=`、`<`、`<=` 和 `>=`）在标准化的过程中已经被降级到 `std::rel_ops` 命名空间中，当时认定它们放在 `std` 命名空间中会带来问题。确实，让这些定义可见会显得任何类都有 `!=` 运算符（其实例化可能会失败），并且该运算符对它的两个实参总是恰好匹配的。虽然这个问题可以通过使用 SFINAE 技巧来解决，也就是说，只有当类型拥有合适的 `==` 运算符时，`!=` 运算符才会为其进行实例化，但是另一个会导致优先匹配的问题还是没有解决：例如，有些用户提供的定义需要进行派生类到基类的转换，那以上一般的 `!=` 定义就会优先被实例化，这可能会造成意料之外的后果。

有一种基于 CRTP 的替代做法，可以让这些运算符定义比一般化的运算符定义更优先被实例化，这样既提高了代码的复用性，又没有将运算符过于一般化的弊病。

```c++
template <typename Derived>
class EqualityComparable {
  public:
    friend bool operator!= (Derived const& x1, Derived const& x2) {
        return !(x1 == x2);
    }
};

class X : public EqualityComparable<X> {
  public:
    friend bool operator== (X const& x1, X const& x2) {
        // 实现比较两个类型为 X 的对象的逻辑
    }
};
```

在此，我们把 CRTP 和 Barton-Nackman 技巧加以组合。`EqualityComparable<>` 使用 CRTP，基于派生类中 `operator==` 的定义为它的派生类提供 `operator!=`。它实际上通过定义一个友元函数（Barton-Nackman 技巧）提供了运算符的定义，而友元函数给了 `operator!=` 的两个形参同样的转换行为。

CRTP 可用于将行为因素纳入基类中同时又保有最终派生类的身份。和 Barton-Nackman 技巧一起使用的时候，CRTP 可以基于几个典型的运算符为众多运算符提供一般性的定义。这些属性使得结合 Barton-Nackman 技巧的 CRTP 称为 C++ 模板库作者钟爱的一种技巧。

### 门面模式

使用 CRTP 和 Barton-Nackman 技巧来定义某些运算符是一条捷径。这个主意更进一步，就可以让 CRTP 基类以 CRTP 派生类公开更小（但也更易于实现）的接口来定义某个类大部分或者全部的公开接口。这种模式被称为门面（facade）模式，它在定义需符合某些现有接口（如数值类型、迭代器、容器等）要求的新类型时尤其管用。

为了说明此模式，我们来实现一个迭代器的门面，它大幅简化了实现遵循标准库要求的迭代器的过程。迭代器类型（尤其是随机访问迭代器）要求实现相当大型的接口。以下类模板 `IteratorFacade` 的基本框架展示了迭代器接口的要求：

```c++
template <typename Derived, typename Value, typename Category,
          typename Reference = Value&, typename Distance = std::ptrdiff_t>
class IteratorFacade {
  public:
    using value_type = typename std::remove_const<Value>::type;
    using reference = Reference;
    using pointer = Value*;
    using difference_type = Distance;
    using iterator_category = Category;

    // 输入迭代器接口
    reference operator *() const { ... }
    pointer operator ->() const { ... }
    Derived& operator ++() { ... }
    Derived operator ++(int) { ... }
    friend bool operator== (IteratorFacade const& lhs, IteratorFacade const& rhs) { ... }
    ...
    // 双向迭代器接口
    Derived& operator --() { ... }
    Derived operator --(int) { ... }

    // 随机访问迭代器接口
    reference operator [](difference_type n) const { ... }
    Derived& operator +=(difference_type n) { ... }
    ...
    friend difference_type operator -(IteratorFacade const& lhs, IteratorFacade const& rhs) { ... }
    friend bool operator <(IteratorFacade const& lhs, IteratorFacade const& rhs) { ... }
    ...
};
```

此处出于简洁，省略了一些声明，但即使为每个新的迭代器实现以上列出的每一个接口，也是相当枯燥的一件事。幸运的是，这些实现可以被提炼成几个核心运算。

- 对于所有迭代器接口实现以下运算。
  - `dereference()`：访问迭代器所指向的值（通常通过 `*` 和 `->` 运算符实现）。
  - `increment()`：移动迭代器以指向序列中的下一个项目。
  - `equals`：判断两个迭代器是否引用了序列中的同一个项目。
- 对于双向迭代器接口实现以下运算。
  - `decrement()`：移动迭代器以指向列表中的前一个项目。
- 对于随机访问迭代器接口实现以下运算。
  - `advance()`：将迭代器向前（或向后）移动 n 步。
  - `measureDistance()`：确定序列中从一个迭代器移到另一个迭代器需要的步数。

门面的作用是对只实现核心操作的类型进行适配以提供完整的迭代器接口。`IteratorFacade` 的实现主要将迭代器的语法映射到最小的接口。在下面的例子中，我们使用成员函数 `asDerived` 来访问 CRTP 派生类：

```c++
Derived& asDerived() {
    return *static_cast<Derived*>(this);
}

Derived const& asDerived() const {
    return *static_cast<Derived const*>(this);
}
```

有了以上定义，该门面的大部分实现就变得直截了当。我们只展示一些满足输入迭代器要求的定义，其他的与此相类似。

```c++
reference operator*() const {
    return asDerived().dereference();
}

Derived& operator++() {
    asDerived().increment();
    return asDerived();
}

Derived operator++(int) {
    Derived result(asDerived());
    asDerived().increment();
    return result;
}

friend bool operator== (IteratorFacade const& lhs, IteratorFacade const& rhs) {
    return lhs.asDerived().equals(rhs.asDerived());
}
```

1. 定义一个链表迭代器

有了 `IteratorFacade` 的定义，我们可以容易地将迭代器定义到简单的链表类中。例如，设想在链表中定义如下节点。

```c++
template <typename T>
class ListNode {
  public:
    T value;
    ListNode<T>* next = nullptr;
    ~ListNode() { delete next; }
};
```

使用 `IteratorFacade`，就能以一种直接的方式把迭代器定义到以上的链表类中。

```c++
template <typename T>
class ListNodeIterator : public IteratorFacade<ListNodeIterator<T>, T, std::forward_iterator_tag> {
    ListNode<T>* current = nullptr;
  public:
    T& dereference() const {
        return current->value;
    }
    void increment() {
        current = current->next;
    }
    bool equals(ListNodeIterator const& other) const {
        return current == other.current;
    }
    ListNodeIterator(ListNode<T>* current = nullptr) : current(current) {}
};
```

`ListNodeIterator` 提供了作为前向迭代器所需的所有正确的运算符和嵌套类型，并且只需要非常少的代码来实现。正如我们将在后面看到的，定义更复杂的迭代器（例如，随机访问迭代器）只需要少量的额外工作。

2. 隐藏接口

以上 `ListNodeIterator` 的一个缺点是，作为公共接口，我们被要求公开 `dereference()`、`advance()` 和 `equals()` 运算。为了消除这一要求，我们可以改造 `IteratorFacade`，通过一个单独的访问类在派生类的 CRTP 类上执行所有的操作，我们称为 `IteratorFacadeAccess`。

```c++
// 将 IteratorFacadeAccess 声明为友元以允许其访问核心迭代器运算
class IteratorFacadeAccess {
    // 只有 IteratorFacade 可以使用这些定义
    template <typename Derived, typename Value, typename Category,
              typename Reference, typename Distance>
    friend class IteratorFacade;

    // 所有迭代器都需要
    template <typename Reference, typename Iterator>
    static Reference dereference(Iterator const& i) {
        return i.dereference();
    }
    ...

    // 双向迭代器需要
    template <typename Iterator>
    static void decrement(Iterator& i) {
        return i.decrement();
    }

    // 随机访问迭代器需要
    template <typename Iterator, typename Distance>
    static void advance(Iterator& i, Distance n) {
        return i.advance(n);
    }
    ...
};
```

该类为每个核心迭代器运算提供静态成员函数，调用所提供迭代器的相应（非静态）成员函数。所有的静态成员函数都是私有的，访问权只授予 `IteratorFacade` 本身，这样一来，我们的 `ListNodeIterator` 可以将 `IteratorFacadeAccess` 作为友元，并将门面所需的接口保持为私有的。

```c++
friend class IteratorFacadeAccess;
```

3. 迭代器适配器

`IteratorFacade` 让我们易于构建像这样的迭代器适配器：它接受一个现有的迭代器并公开一个新迭代器，从而可以为底层序列提供视角转换。例如，我们可能有一个 `Person` 值的容器。

```c++
struct Person {
    std::string firstName;
    std::string lastName;

    friend std::ostream& operator<<(std::ostream& strm, Person const& p) {
        return strm << p.lastName << ", " << p.firstName;
    }
};
```

然而，我们并不想在容器中迭代所有的 `Person` 值，而是只想看到人们的名字。在这里，我们介绍名为 `ProjectionIterator` 的迭代器适配器，它允许我们将底层（基）迭代器的值“投射”到一些指向数据的指针成员上，如 `Person::firstName`。

`ProjectionIterator` 是一个迭代器，由基迭代器（Iterator）和将由迭代器公开的值的类型（`T`）来定义。

```c++
template <typename Iterator, typename T>
class ProjectionIterator : public IteratorFacade<ProjectionIterator<Iterator, T>,
                                                 T,
                                                 typename std::iterator_traits<Iterator>::iterator_category,
                                                 T&,
                                                 typename std::iterator_traits<Iterator>::difference_type> {
    using Base = typename std::iterator_traits<Iterator>::value_type;
    using Distance = typename std::iterator_traits<Iterator>::difference_type;

    Iterator iter;
    T Base::* member;

    friend class IteratorFacadeAccess;
    ... // 为 IteratorFacade 实现核心运算

  public:
    ProjectionIterator(Iterator iter, T Base::* member)
        : iter(iter), member(member) {}
};

template <typename Iterator, typename Base, typename T>
auto project(Iterator iter, T Base::* member) {
    return ProjectionIterator<Iterator, T>(iter, member);
}
```

以上模板定义的每个投射迭代器都存储了两个值：`iter` 是进入底层（`Base` 值的）序列的迭代器，而 `member` 是一个指向数据的指针成员，描述要对哪个成员进行投射。由此，我们来考虑提供给 `IteratorFacade` 基类的模板参数。第 1 个参数是 `ProjectionIterator` 本身（以启用 CRTP）。第 2 个（`T`）和第 4 个（`T&`）参数分别是我们的投射迭代器的值类型和引用类型，这就把投射迭代器定义为一个 `T` 值的序列。第 3 个和第 5 个参数只是传递底层迭代器的类别以及不同类型。因此，当 `Iterator` 是输入迭代器时，投射迭代器将是输入迭代器；当 `Iterator` 是双向迭代器时，投射迭代器就是双向迭代器。`project()` 函数让投射迭代器易于构建。

现在就只缺少 `IteratorFacade` 的核心要求的实现了。值得注意的是，`dereference` 将底层迭代器解引用，然后通过指向数据的指针成员进行投射。

```c++
T& dereference() const {
    return (*iter).*member;
}
```

其余运算通过底层迭代器实现。

```c++
void increment() {
    ++iter;
}

bool equals(ProjectionIterator const& other) const {
    return iter == other.iter;
}

void decrement() {
    --iter;
}
```

随机访问迭代器的定义可以此类推，出于简洁，在此省略。

这就是了！有了投射迭代器，我们可以输出某个用来装 `Person` 值的 `vector` 里的人们的名字：
