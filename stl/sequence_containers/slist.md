# slist

```c++
// 单向链表的节点基本结构
struct __slist_node_base {
    __slist_node_base* next;
};

// 单向链表的节点结构
template <class T>
struct __slist_node : public __slist_node_base {
    T data;
};

// 全局函数：已知某一节点，插入新节点于其后
inline __slist_node_base* __slist_make_link(__slist_node_base* prev_node, __slist_node_base* new_node) {
    // 令new节点的下一节点为prev节点的下一节点
    new_node->next = prev_node->next;
    prev_node->next = new_node;     // 令prev节点的下一节点指向new节点
    return new_node;
}

// 全局函数：单向链表的大小（元素个数）
inline size_t __slist_size(__slist_node_base* node) {
    size_t result = 0;
    for (; node != 0; node = node->next)
        ++result;       // 一个一个累计
    return result;
}

// 单向链表的迭代器基本结构
struct __slist_iterator_base
{
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef forward_iterator_tag iterator_category; // 注意，单向

    __slist_node_base* node;    // 指向节点基本结构

    __slist_iterator_base(__slist_node_base* x) : node(x) {}

    void incr() { node = node->next; }  // 前进一个节点

    bool operator==(const __slist_iterator_base& x) const {
        return node == x.node;
    }
    bool operator!=(const __slist_iterator_base& x) const {
        return node != x.node;
    }
};

// 单向链表的迭代器结构
template <class T, class Ref, class Ptr>
struct __slist_iterator : public __slist_iterator_base
{
    typedef __slist_iterator<T, T&, T*>     iterator;
    typedef __slist_iterator<T, const T&, const T*> const_iterator;
    typedef __slist_iterator<T, Ref, Ptr>   self;

    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef __slist_node<T> list_node;

    __slist_iterator(list_node* x) : __slist_iterator_base(x) {}
    // 调用slist<T>::end()时会造成__slist_iterator(0)，于是调用上述函数
    __slist_iterator() : __slist_iterator_base(0) {}
    __slist_iterator(const iterator& x) : __slist_iterator_base(x.node) {}

    reference operator*() const { return ((list_node*) node)->data; }
    pointer operator->() const { return &(operator*()); }

    self& operator++() {
        incr(); // 前进一个节点
        return *this;
    }
    self operator++(int) {
        self tmp = *this;
        incr(); // 前进一个节点
        return tmp;
    }

// 没有实现operator--，因为这是一个forward iterator
};

template <class T, class Alloc = alloc>
class slist
{
public:
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef __slist_iterator<T, T&, T*> iterator;
    typedef __slist_iterator<T, const T&, cosnt T*> const_iterator;

private:
    typedef __slist_node<T> list_node;
    typedef __slist_node_base list_node_base;
    typedef __slist_iterator_base iterator_base;
    typedef simple_alloc<list_node, Alloc> list_node_allocator;

    static list_node* create_node(const value_type& x) {
        list_node* node = list_node_allocator::allocate();  // 配置空间
        __STL_TRY {
            construct(&node->data, x);      // 构造元素
            node->next = 0;
        }
        __STL_UNWIND(list_node_allocator::deallocate(node));
        return node;
    }

    static void destroy_node(list_node* node) {
        destroy(&node->data);       // 将元素析构
        list_node_allocator::deallocate(node);  // 释放空间
    }

private:
    list_node_base head;    // 头部。注意，它不是指针，是实物

public:
    slist() { head.next = 0; }
    ~slist() { clear(); }

public:
    iterator begin() { return iterator((list_node*)head.next); }
    iterator end() { return iterator(0); }
    size_type size() const { return __slist_size(head.next); }
    bool empty() const { return head.next == 0; }

    // 两个slist互换：只要将head交换互指即可
    void swap(slist& L) {
        list_node_base* tmp = head.next;
        head.next = L.head.next;
        L.head.next = tmp;
    }

public:
    // 取头部元素
    reference front() { return ((list_node*) head.next)->data; }

    // 从头部插入元素（新元素成为slist的第一个元素）
    void push_front(const value_type& x) {
        __slist_make_list(&head, create_node(x));
    }

    // 注意，没有push_back()

    // 从头部取走元素（删除之）。修改head
    void pop_front() {
        list_node* node = (list_node*)head.next;
        head.next = node->next;
        destroy_node(node);
    }
...
};
```
