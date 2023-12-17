# adapters

## container adapters

```c++
template <class T, class Sequence = deque<T>>
class stack {
protected:
    Sequence c;     // 底层容器
    ...
};

template <class T, class Sequence = deque<T>>
class queue {
protected:
    Sequence c;     // 底层容器
    ...
};
```

## iterator adapters

### insert iterators

```c++
// 这是一个迭代器配接器（iterator adapter），用来将某个迭代器的赋值（assign）
// 操作修改为插入（insert）操作————从容器的尾端插入进去（所以称为back_insert）
template <class Container>
class back_insert_iterator {
protected:
    Container* container;       // 底层容器
public:
    typedef output_iterator_tag iterator_category;      // 注意类型
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    // 下面这个 ctor 使 back_insert_iterator 与容器绑定起来
    explicit back_insert_iterator(Container& x) : container(&x) {}
    back_insert_iterator<Container>& operator=(const typename Container::value_type& value) {
        container->push_back(value);    // 这里是关键，转而调用 push_back()
        return *this;
    }

    // 以下三个操作符对 back_insert_iterator 不起作用（关闭功能）
    // 三个操作符返回的都是 back_insert_iterator 自己
    back_insert_iterator<Container>& operator*() { return *this; }
    back_insert_iterator<Container>& operator++() { return *this; }
    back_insert_iterator<Container>& operator++(int) { return *this; }
};

// 这是一个辅助函数，帮助我们方便使用 back_insert_iterator
template <class Container>
inline back_insert_iterator<Container> back_inserter(Container& x) {
    return back_insert_iterator<Container>(x);
}

//-----------------------------------------------------------------
// 这是一个迭代器配接器（iterator adapter），用来将某个迭代器的赋值（assign）
// 操作修改为插入（insert）操作————从容器的头端插入进去（所以称为front_insert）
// 注意，该迭代器不适用于 vector ，因为 vector 没有提供 push_front 函数
template <class Container>
class front_insert_iterator {
protected:
    Container* container;       // 底层容器
public:
    typedef output_iterator_tag iterator_category;      // 注意类型
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    explicit front_insert_iterator(Container& x) : container(&x) {}
    front_insert_iterator<Container>& operator=(const typename Container::value_type& value) {
        container->push_back(value);    // 这里是关键，转而调用 push_front()
        return *this;
    }

    // 以下三个操作符对 front_insert_iterator 不起作用（关闭功能）
    // 三个操作符返回的都是 front_insert_iterator 自己
    front_insert_iterator<Container>& operator*() { return *this; }
    front_insert_iterator<Container>& operator++() { return *this; }
    front_insert_iterator<Container>& operator++(int) { return *this; }
};

// 这是一个辅助函数，帮助我们方便使用 front_insert_iterator
template <class Container>
inline front_insert_iterator<Container> front_inserter(Container& x) {
    return front_insert_iterator<Container>(x);
}

//-----------------------------------------------------------------
// 这是一个迭代器配接器（iterator adapter），用来将某个迭代器的赋值（assign）
// 操作修改为插入（insert）操作，在指定的位置上进行，并将迭代器右移一个位置
// ————如此便可很方便地连续执行“表面上是赋值（覆写）而实际上是插入”的操作
template <class Container>
class insert_iterator {
protected:
    Container* container;       // 底层容器
    typename Container::iterator iter;
public:
    typedef output_iterator_tag iterator_category;      // 注意类型
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    insert_iterator(Container& x, typename Container::iterator i) : container(&x), iter(i) {}
    insert_iterator<Container>& operator=(const typename Container::value_type& value) {
        iter = container->insert(iter, value);    // 这里是关键，转而调用 insert()
        ++iter; // 注意这个，使 insert iterator 永远随其目标贴身移动
        return *this;
    }

    // 以下三个操作符对 insert_iterator 不起作用（关闭功能）
    // 三个操作符返回的都是 insert_iterator 自己
    insert_iterator<Container>& operator*() { return *this; }
    insert_iterator<Container>& operator++() { return *this; }
    insert_iterator<Container>& operator++(int) { return *this; }
};

// 这是一个辅助函数，帮助我们方便使用 insert_iterator
template <class Container, class Iterator>
inline insert_iterator<Container> inserter(Container& x, Iterator i) {
    typedef typename Container::iterator iter;
    return insert_iterator<Container>(x, iter(i));
}
```

### reverse iterators

```c++
// 这是一个迭代器配接器（iterator adapter），用来将某个迭代器逆反前进方向，
// 使前进为后退，后退为前进
template <class Iterator>
class reverse_iterator {
protected:
    Iterator current;   // 记录对应之正向迭代器
public:
    // 逆向迭代器的5种相应型别（associated types）都和其对应的正向迭代器相同
    typedef typename iterator_traits<Iterator>::iterator_category iterator_category;
    typedef typename iterator_traits<Iterator>::value_type value_type;
    typedef typename iterator_traits<Iterator>::difference_type difference_type;
    typedef typename iterator_traits<Iterator>::pointer pointer;
    typedef typename iterator_traits<Iterator>::reference reference;

    typedef Iterator iterator_type;     // 代表正向迭代器
    typedef reverse_iterator<Iterator> self;    // 代表反向迭代器

public:
    reverse_iterator() {}
    // 下面这个 ctor 将 reverse_iterator 与某个迭代器x系结起来
    explicit reverse_iterator(iterator_type x) : current(x) {}
    reverse_iterator(const self& x) : current(x.current) {}

    iterator_type base() const { return current; }  // 取出对应的正向迭代器
    reference operator*() const {
        Iterator tmp = current;
        return *--tmp;
        // 以上为关键所在。对逆向迭代器取值，就是将“对应之正向迭代器”后退一格而后取值
    }
    pointer operator->() const { return &(operator*()); }   // 意义同上

    // 前进（++）变成后退（--）
    self& operator++() {
        --current;
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        --current;
        return tmp;
    }
    // 后退（--）变成前进（++）
    self& operator--() {
        ++current;
        return *this;
    }
    self operator--(int) {
        self tmp = *this;
        ++current;
        return tmp;
    }
    // 前进与后退方向完全逆转
    self operator+(difference_type n) const {
        return self(current - n);
    }
    self& operator+=(difference_type n) {
        current -= n;
        return *this;
    }
    self operator-(difference_type n) const {
        return self(current + n);
    }
    self& operator-=(difference_type n) {
        current += n;
        return *this;
    }
    // 注意，下面第一个 * 和唯一一个 + 都会调用本类的 operator* 和 operator+，
    // 第二个 * 则不会。（判断法则：完全看处理的型别是什么而定）
    reference operator[] (difference_type n) const { return *(*this + n); }
};
```

### stream iterators

```c++
// 这是一个 input iterator，能够为“来自某一 basic_istream”的对象执行
// 格式化输入操作。注意，此版本为旧有之HP规格，未符合标准接口：
// istream_iterator<T, charT, traits, Distance>
// 然而一般使用 input iterators 时都只使用第一个 template 参数，此时以下仍适用
// 注：SGI STL 3.3已实现出符合标准接口的 istream_iterator。做法与本版大同小异
// 本版可读性较高
template <class T, class Distance = ptrdiff_t>
class istream_iterator {
    friend bool operator== __STL_NULL_TMPL_ARGS (const istream_iterator<T, Distance>& x,
                                                 const istream_iterator<T, Distance>& y);
    // 以上语法很奇特，请参考《C++ Primer》3e, p834, bound friend function template

protected:
    istream* stream;
    T value;
    bool end_marker;
    void read() {
        end_marker = (*stream) ? true : false;
        if (end_marker) *stream >> value;
        // 以上，输入之后，stream 的状态可能改变，所以下面再判断一次以决定 end_marker
        // 当读到 eof 或读到型别不符的资料，stream 即处于 false 状态
        end_marker = (*stream) ? true : false;
    }
public:
    typedef input_iterator_tag  iterator_category;
    typedef T                   value_type;
    typedef Distance            difference_type;
    typedef const T*            pointer;
    typedef const T&            reference;
    // 以上，因身为 input iterator，所以采用 const 比较保险

    istream_iterator() : stream(&cin), end_marker(false) {}
    istream_iterator(istream& s) : stream(&s) { read(); }
    // 以上两行的用法：
    // istream_iterator<int> eos;       造成 end_marker 为 false。
    // istream_iterator<int> initer(cin);   引发 read()。程序至此会等待输入
    // 因此，下面这两行客户端程序：
    // istream_iterator<int> initer(cin);   (A)
    // cout << "please input..." << endl;   (B)
    // 会停留在(A)等待一个输入，然后才执行(B)出现提示信息。这是不合理的现象
    // 规避之道：永远在最必要的时候，才定义一个 istream_iterator。

    reference operator*() const { return value; }
    pointer operator->() const { return &(operator*()); }

    // 迭代器前进一个位置，就代表要读取一笔资料
    istream_iterator<T, Distance>& operator++() {
        read();
        return *this;
    }
    istream_iterator<T, Distance> operator++(int) {
        istream_iterator<T, Distance> tmp = *this;
        read();
        return tmp;
    }
};

// 这是一个 output iterator，能够将对象格式化输出到某个 basic_istream 上
// 注意，此版本为旧有之HP规格，未符合标准接口：
// ostream_iterator<T, charT, traits>
// 然而一般使用 output iterators 时都只使用第一个 template 参数，此时以下仍适用
// 注：SGI STL 3.3已实现出符合标准接口的 ostream_iterator。做法与本版大同小异
// 本版可读性较高
template <class T, class Distance = ptrdiff_t>
class ostream_iterator {
protected:
    ostream* stream;
    const char* string;     // 每次输出后的间隔符号
                            // 变量名称为 string 可以吗？可以！
public:
    typedef output_iterator_tag iterator_category;
    typedef void                value_type;
    typedef void                difference_type;
    typedef void                pointer;
    typedef void                reference;

    ostream_iterator(ostream& s) : stream(&s), string(0) {}
    ostream_iterator(ostream& s, const char* c) : stream(&s), string(c) {}
    // 以上 ctors 的用法：
    // ostream_iterator<int> outiter(cout, ' '); 输出至 cout，每次间隔一空格

    // 对迭代器做赋值（assign）操作，就代表要输出一笔资料
    ostream_iterator<T>& operator=(const T& value) {
        *stream << value;               // 关键：输出数值
        if (string) *stream << string;  // 如果间隔符号不为空，输出间隔符号
        return *this;
    }
    // 注意以下三个操作
    ostream_iterator<T>& operator*() { return *this; }
    ostream_iterator<T>& operator++() { return *this; }
    ostream_iterator<T>& operator++(int) { return *this; }
};
```

## function adapters

### not1, not2

### bind1st, bind2nd

```c++
// 以下配接器用来将某个 Adaptable Binary function 转换为 Unary Function
template <class Operation>
class binder1st : public unary_function<typename Operation::second_argument_type,
                                        typename Operation::result_type> {
protected:
    Operation op;   // 内部成员
    typename Operation::first_argument_type value;  // 内部成员

public:
    // constructor
    binder1st(const Operation& x,
              const typename Operation::first_argument_type& y)
        : op(x), value(y) {}    // 将表达式和第一参数记录于内部成员
    
    typename Operation::result_type
    operator()(const typename Operation::second_argument_type& x) const {
        return op(value, x);    // 实际调用表达式，并将 value 绑定为第一参数
    }
};

// 辅助函数，让我们得以方便使用 binder1st<Op>
template <class Operation, class T>
inline binder1st<Operation> bind1st(const Operation& op, const T& x) {
    typedef typename Operation::first_argument_type arg1_type;
    return binder1st<Operation>(op, arg1_type(x));
        // 以上，注意，先把x转型为op的第一参数型别
}

//---------------------------------------------------------------
// 以下配接器用来将某个 Adaptable Binary function 转换为 Unary Function
template <class Operation>
class binder2nd : public unary_function<typename Operation::first_argument_type,
                                        typename Operation::result_type> {
protected:
    Operation op;   // 内部成员
    typename Operation::second_argument_type value; // 内部成员
public:
    // constructor
    binder2nd(const Operation& x,
            const typename Operation::second_argument_type& y)
        : op(x), value(y) {}    // 将表达式和第一参数记录于内部成员
    
    typename Operation::result_type
    operator()(const typename Operation::first_argument_type& x) const {
        return op(x, value);    // 实际调用表达式，并将 value 绑定为第二参数
    }
};

// 辅助函数，让我们得以方便使用 binder2nd<Op>
template <class Operation, class T>
inline binder2nd<Operation> bind2nd(const Operation& op, const T& x) {
    typedef typename Operation::second_argument_type arg2_type;
    return binder2nd<Operation>(op, arg2_type(x));
        // 以上，注意，先把x转型为op的第二参数型别
}
```

### compose1, compose2

### ptr_fun

### mem_fun, mem_fun_ref
