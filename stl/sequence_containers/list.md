# list

```cpp
template <class T>
struct __list_node {
    typedef void* void_pointer;
    void_pointer prev;  // 型别为 void*。其实可设为 __list_node<T>*
    void_pointer next;
    T data;
}
```

```cpp
template<class T, class Ref, class Ptr>
struct __list_iterator {
    typedef __list_iterator<T, T&, T*>      iterator;
    typedef __list_iterator<T, Ref, Ptr>    self;

    typedef bidirectional_iterator_tag iterator_category;
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef __list_node<T>* link_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    link_type node; // 迭代器内部当然要有一个普通指针，指向list的节点

    // constructor
    __list_iterator(link_type x) : node(x) {}
    __list_iterator() {}
    __list_iterator(const iterator& x) : node(x.node) {}

    bool operator==(const self* x) const { return node == x.node; }
    bool operator!=(const self* x) const { return node != x.node; }
    // 以下对迭代器取值（dereference），取的是节点的数据值
    reference operator*() const { return (*node).data; }

    // 以下是迭代器的成员存取（member access）运算子的标准做法
    pointer operator->() const { return &(operator*()); }
    // 对迭代器累加1，就是前进一个节点
    self& operator++() {
        node = (link_type)((*node).next);
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        ++*this;
        return tmp;
    }

    // 对迭代器递减1，就是后退一个节点
    self& operator--() {
        node = (link_type)((*node).prev);
        return *this;
    }
    self operator--(int) {
        self tmp = *this;
        --*this;
        return tmp;
    }
}
```

```cpp
template <class T, class Alloc = alloc>
class list {
protected:
    typedef __list_node<T> list_node;
    // 专属之空间配置器，每次配置一个节点大小
    typedef simple_alloc<list_node, Alloc> list_node_allocator;
public:
    typedef list_node* link_type;

protected:
    link_type node; // 只要一个指针，便可表示整个环状双向链表

public:
    iterator begin() { return (link_type)((*node).next); }
    iterator end() { return node; }
    bool empty() const { return node->next == node; }
    size_type size() const {
        size_type result = 0;
        distance(begin(), end(), result);   // 全局函数
        return result;
    }
    // 取头结点的内容
    reference front() { return *begin(); }
    // 取尾结点的内容
    reference back() { return *(--end()); }

protected:
    // 配置一个节点并传回
    link_type get_node() { return list_node_allocator::allocate(); }
    // 释放一个节点
    void put_node(link_type p) { list_node_allocator::deallocate(p); }

    // 产生（配置并构造）一个节点，带有元素值
    link_type create_node(const T& x) {
        link_type p = get_node();
        construct(&p->data, x); // 全局函数，构造/析构基本工具
        return p;
    }
    // 销毁（析构并释放）一个节点
    void destroy_node(link_type p) {
        destroy(&p->data);      // 全局函数，构造/析构基本工具
        put_node(p);
    }

public:
    list() { empty_initialize(); }  // 产生一个空链表

protected:
    void empty_initialize() {
        node = get_node();  // 配置一个节点空间，令node指向它
        node->next = node;  // 令node头尾都指向自己，不设元素值
        node->prev = node;
    }

public:
    // 函数目的：在迭代器position所指位置插入一个节点，内容为x
    iterator insert(iterator position, const T& x) {
        link_type tmp = create_node(x); // 产生一个节点（设妥内容为x）
        // 调整双向指针，使tmp插入进去
        tmp->next = position.node;
        tmp->prev = position.node->prev;
        (link_type(position.node->prev))->next = tmp;
        position.node->prev = tmp;
        return tmp;
    }

public:
    // 插入一个节点，作为头结点
    void push_front(const T& x) { insert(begin(), x); }
    // 插入一个节点，作为尾结点
    void push_back(const T& x) { insert(end(), x); }

    // 移除迭代器position所指节点
    iterator erase(iterator position) {
        link_type next_node = link_type(position.node->next);
        link_type prev_node = link_type(position.node->prev);
        prev_node->next = next_node;
        next_node->prev = prev_node;
        destroy_node(position.node);
        return iterator(next_node);
    }

    // 移除头结点
    void pop_front() { erase(begin()); }
    // 移除尾结点
    void pop_back() {
        iterator tmp = end();
        erase(--tmp);
    }

protected:
    // 将[first, last]内的所有元素移动到position之前
    void transfer(iterator position, iterator first, iterator last) {
        if (position != last) {
            (*(link_type((*last.node).prev))).next = position.node;
            (*(link_type((*first.node).prev))).next = last.node;
            (*(link_type((*position.node).prev))).next = first.node;
            link_type tmp = link_type((*position.node).prev);
            (*position.node).prev = (*last.node).prev;
            (*last.node).prev = (*first.node).prev;
            (*first.node).prev = tmp;
        }
    }

public:
    // 将x接合于position所指位置之前。x必须不同于*this
    void splice(iterator position, list& x) {
        if (!x.empty())
            transfer(position, x.begin(), x.end());
    }

    // 将i所指元素接合于position所指位置之前。position和i可指向同一个list
    void splice(iterator position, list&, iterator i) {
        iterator j = i;
        ++j;
        if (position == i || position == j) return;
        transfer(position, i, j);
    }

    // 将[first, last]内的所有元素接合于position所指位置之前
    // position和[first, last]可指向同一个list，
    // 但position不能位于[first, last]之内
    void splice(iterator position, list&, iterator first, iterator last) {
        if (first != last)
            transfer(position, first, last);
    }
};
```

```cpp
// 清除所有节点
template <class T, class Alloc>
void list<T, Alloc>::clear()
{
    link_type cur = (link_type) node->next; // begin()
    while (cur != node) {       // 遍历每一个节点
        link_type = tmp = cur;
        cur = (link_type) cur->next;
        destroy_node(tmp);      // 销毁（析构并释放）一个节点
    }
    // 恢复node原始状态
    node->next = node;
    node->prev = node;
}

// 将数值为value之所有元素移除
template <class T, class Alloc>
void list<T, Alloc>::remove(const T& value) {
    iterator first = begin();
    iterator last = end();
    while (first != last) {     // 遍历每一个节点
        iterator next = first;
        ++next;
        if (*first == value) erase(first);  // 找到就移除
        first = next;
    }
}

// 移除数值相同的连续元素。注意，只有“连续而相同的元素”，才会被移除剩一个
template <class T, class Alloc>
void list<T, Alloc>::unique() {
    iterator first = begin();
    iterator last = end();
    if (first == last)  return;     // 空链表，什么都不必做
    iterator next = first;
    while (++next != last) {        // 遍历每一个节点
        if (*first == *next)        // 如果在此区段中有相同的元素
            erase(next);            // 移除之
        else
            first = next;           // 调整指针
        next = first;               // 修正区段范围
    }
}

// merge()将x合并到*this身上。两个lists的内容都必须先经过递增排序
template <class T, class Alloc>
void list<T, Alloc>::merge(list<T, Alloc>& x) {
    iterator first1 = begin();
    iterator last1 = end();
    iterator first2 = x.begin();
    iterator last2 = x.end();

    // 注意：前提是，两个lists都已经过递增排序
    while (first1 != last1 && first2 != last2)
        if (*first2 < *first1) {
            iterator next = first2;
            transfer(first1, first2, ++next);
            first2 = next;
        } else
            ++first1;
    if (first2 != last2) transfer(last1, first2, last2);
}

// reverse()将*this的内容逆向重置
template <class T, class Alloc>
void list<T, Alloc>::reverse() {
    // 以下判断，如果是空链表，或仅有一个元素，就不进行任何操作
    // 使用size() == 0 || size() == 1来判断，虽然也可以，但是比较慢
    if (node->next == node || link_type(node->next)->next == node)
        return;
    iterator first = begin();
    ++first;
    while (first != end()) {
        iterator old = first;
        ++first;
        transfer(begin(), old, first);
    }
}

// list不能使用STL算法sort()，必须使用自己的sort() member function，
// 因为STL算法sort()只接受RamdonAccessIterator
// 本函数采用quick sort
template <class T, class Alloc>
void list<T, Alloc>::sort() {
    // 以下判断，如果是空链表，或仅有一个元素，就不进行任何操作
    // 使用size() == 0 || size() == 1来判断，虽然也可以，但是比较慢
    if (node->next == node || link_type(node->next)->next == node)
        return;

    // 一些新的lists，作为中介数据存放区
    list<T, Alloc> carry;
    list<T, Alloc> counter[64];
    int fill = 0;
    while (!empty()) {
        carry.splice(carry.begin(), *this, begin());
        int i = 0;
        while (i < fill & !counter[i].empty()) {
            counter[i].merge(carry);
            carry.swap(counter[i++]);
        }
        carry.swap(counter[i]);
        if (i == fill) ++fill;
    }

    for (int i=1; i<fill; ++i) {
        counter[i].merge(counter[i-1]);
        swap(counterp[fill - 1]);
    }
}
```
