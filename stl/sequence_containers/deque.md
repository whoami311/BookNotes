# deque

```c++
template<class T, class Ref, class Ptr, size_t BufSiz>
struct __deque_iterator {   // 未继承std::iterator
    typedef __deque_iterator<T, T&, T*, BufSiz>     iterator;
    typedef __deque_iterator<T, const T&, const T*, BufSiz> const_iterator;
    static size_t buffer_size() { return __deque_buf_size(BufSiz, sizeof(T)); }

    // 未继承std::iterator，所以必须自行撰写五个必要的迭代器相应型别
    typedef random_access_iterator_tag iterator_category;
    typedef T value_type;
    typedef Ptr pointer;
    typedef Ref reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    typedef T** map_pointer;

    typedef __deque_iterator self;

    // 保持与容器的联结
    T* cur;     // 此迭代器所指之缓冲区中的现行（current）元素
    T* first;   // 此迭代器所指之缓冲区的头
    T* last;    // 此迭代器所指之缓冲区的尾（含备用空间）
    map_pointer node;   // 指向管控中心
...
}

// 如果n不为0，传回n，表示buffer size由用户自定义
// 如果n为0，表示buffer size使用默认值，那么
//   如果sz（元素大小，sizeof(value_type)）小于512，传回512/sz，
//   如果sz不小于512，传回1
inline size_t __deque_buf_size(size_t n, size_t sz)
{
    return n != 0 ? n : (sz < 512 ? size_t(512 / sz) : size_t(1));
}

void set_node(map_pointer new_node) {
    node = new_node;
    first = *new_node;
    last = first + difference_type(buffer_size());
}

reference operator*() const { return *cur; }
pointer operator->() const { return &(operator*()); }
difference_type operator-(const self& x) const {
    return difference_type(buffer_size()) * (node - x.node - 1) + (cur - first) + (x.last - x.cur);
}

// postfix forms of increment and decrement operators
self& operator++() {
    ++cur;                  // 切换至下一个元素
    if (cur == last) {      // 如果已达所在缓冲区的尾端
        set_node(node + 1); // 就切换至下一节点（亦即缓冲区）
        cur = first;        // 的第一个元素
    }
    return *this;
}
self operator++(int) {      // 后置式，标准写法
    self tmp = *this;
    ++*this;
    return tmp;
}
self& operator--() {
    if (cur == first) {     // 如果已达所在缓冲区的头端
        set_node(node - 1); // 就切换至前一节点（亦即缓冲区）
        cur = last;         // 的最后一个元素（的下一位置）
    }
    --cur;                  // 切换至前一个元素
    return *this;
}
self operator--(int) {      // 后置式，标准写法
    self tmp = *this;
    --*this;
    return tmp;
}

// 以下实现随机存取。迭代器可以直接跳跃n个距离
self& operator+=(difference_type n) {
    difference_type offset = n + (cur - first);
    if (offset >= 0 && offset < difference_type(buffer_size()))
        // 目标位置在同一缓冲区
        cur += n;
    else {
        // 目标的位置不在同一缓冲区内
        difference_type node_offset = offset > 0 ? offset / difference_type(buffer_size()) : -difference_type((-offset - 1) / buffer_size()) - 1;
        // 切换至正确的节点（亦即缓冲区）
        set_node(node + node_offset);
        // 切换至正确的位置
        cur = first + (offset - node_offset * difference_type(buffer_size()));
    }
    return *this;
}

// stand-alone op
self operator+(difference_type n) const {
    self tmp = *this;
    return tmp += n;    // 调用operator+=
}

// 利用operator+=来完成operator-=
self& operator-=(difference_type n) { return *this += -n; }

self operator-(difference_type n) const {
    self tmp = *this;
    return tmp -= n;    // 调用operator-=
}

// 以下实现随机存取。迭代器可以直接跳跃n个距离
// 调用operator*，operator+
reference operator[](difference_type n) const { return *(*this + n); }

bool operator==(const self& x) const { return cur == x.cur; }
bool operator!=(const self& x) const { return !(*this == x); }
bool operator<(const self& x) const {
    return (node == x.node) ? (cur < x.cur) : (node < x.node);
}
```

```c++
// 见__deque_buf_size()。BufSize默认值为0的唯一理由是为了闪避某些
// 编译器在处理常数算式（constant expressions）时的臭虫
template <class T, class Alloc = alloc, size_t BufSiz = 0>
class deque {
public:                         // Basic types
    typedef T value_type;
    typedef vlaue_type* pointer;
    typedef size_t size_type;

public:                         // Iterators
    typedef __deque_iterator<T, T&, T*, BufSiz> iterator;

protected:
    // 元素的指针的指针（pointer of pointer of T）
    typedef pointer* map_pointer;

protected:                  // Data members
    iterator start;         // 表现第一个节点
    iterator finish;        // 表现最后一个节点

    map_pointer map;        // 指向map，map是块连续空间
                            // 其每个元素都是个指针，指向一个节点（缓冲区）
    size_type map_size;     // map内有多少指针

public:                     // Basic members
    iterator begin() { return start; }
    iterator end() { return finish; }

    reference operator[](size_type n) {
        return start[difference_type(n)];   // 调用__deque_iterator<>::operator[]
    }

    reference front() { return *start; }    // 调用__deque_iterator<>::operator*
    reference back() {
        iterator tmp = finish;
        --tmp;      // 调用__deque_iterator<>::operator--
        return *tmp;    // 调用__deque_iterator<>::operator*
        // 以上三行何不改为：return *(finish - 1);
        // 因为__deque_iterator<>没有为（finish-1）定义运算子?!
    }

    // 下行最后有两个‘;’，虽奇怪但合乎语法
    size_type size() const { return finish - start;; }
    // 以上调用iterator::operator-
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return finish == start; }

protected:              // Internal typedefs
    // 专属之空间配置器，每次配置一个元素大小
    typedef simple_alloc<value_type, Alloc> data_allocator;
    // 专属之空间配置器，每次配置一个指针大小
    typedef simple_alloc<pointer, Alloc> map_allocator;

public:
    deque(int n, const value_type& value) : start(), finish(), map(0), map_size(0)
    {
        fill_initialize(n, value);
    }

public:
    push_back(const value_type& t) {
        if (finish.cur != finish.last - 1) {
            // 最后缓冲区尚有两个（含）以上的元素备用空间
            construct(finish.cur, t);   // 直接在备用空间上构造元素
            ++finish.cur;               // 调整最后缓冲区的使用状态
        }
        else    // 最后缓冲区只剩一个元素备用空间
            push_back_aux(t);
    }

    push_front(const value_type& t) {
        if (start.cur != start.first) {     // 第一缓冲区尚有备用空间
            consturct(start.cur - 1, t);    // 直接在备用空间上构造元素
            --start.cur;        // 调整第一缓冲区的使用状态
        }
        else            // 第一缓冲区已无备用空间
            push_front_aux(t);
    }

    void reserve_map_at_back(size_type nodes_to_add = 1) {
        if (nodes_to_add + 1 < map_size - (finish.node - map))
            // 如果map尾端的节点备用空间不足
            // 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
            reallocate_map(nodes_to_add, false);
    }

    void reserve_map_at_front(size_type nodes_to_add = 1) {
        if (nodes_to_add > start.node - map)
            // 如果map前端的节点备用空间不足
            // 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
            reallocate_map(nodes_to_add, true);
    }

    void pop_back() {
        if (finish.cur != finish.first) {
            // 最后缓冲区有一个（或更多）元素
            --finish.cur;               // 调整指针，相当于排除了最后元素
            destroy(finish.cur);        // 将最后元素析构
        }
        else
            // 最后缓冲区没有任何元素
            pop_back_aux();             // 这里将进行缓冲区的释放工作
    }

    void pop_front() {
        if (start.cur != start.last - 1) {
            // 第一缓冲区有两个（或更多）元素
            destroy(start.cur);     // 将第一元素析构
            ++start.cur;            // 调整指针，相当于排除了第一元素
        } else
            // 第一缓冲区仅有一个元素
            pop_front_aux();        // 这里将进行缓冲区的释放工作
    }

    iterator erase(iterator pos) {
        iterator next = pos;
        ++next;
        difference_type index = pos - start;    // 清除点之前的元素个数
        if (index < (size() >> 1)) {            // 如果清除点之前的元素比较少，
            copy_backward(start, pos, next);    // 就移动清除点之前的元素
            pop_front();                // 移动完毕，最前一个元素冗余，去除之
        } else {                          // 清除点之后的元素比较少，
            copy(next, finish, pos);    // 就移动清除点之后的元素
            pop_back();                 // 移动完毕，最后一个元素冗余，去除之
        }
        return start + index;
    }

    // 在position处插入一个元素，其值为x
    iterator insert(iterator position, const value_type& x) {
        if (position.cur == start.cur) {    // 如果插入点是deque最前端
            push_front(x);                  // 交给push_front去做
            return start;
        } else if (position.cur == finish.cur) {    // 如果插入点是deque最尾端
            push_back(x);                           // 交给push_back去做
            iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            return insert_aux(position, x);         // 交给insert_aux去做
        }
    }
};

template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::fill_initialize(size_type n, const value_type& value) {
    create_map_and_nodes(n);    // 把deque的结构都产生并安排好
    map_pointer cur;
    __STL_TRY {
        // 为每个节点的缓冲区设定初值
        for (cur = start.node; cur < finish.node; ++cur)
            uninitialized_fill(*cur + buffer_size(), value);
        // 最后一个节点的设定稍有不同（因为尾端可能有备用空间，不必设初值）
        uninitialized_fill(finish.first, finish.cur, value);
    }
    catch(...) {
        ...
    }
}

template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::create_map_and_nodes(size_type num_elements) {
    // 需要节点数=（元素个数/每个缓冲区可容纳的元素个数）+1
    // 如果刚好整除，会多配一个节点
    size_type num_nodes = num_elements / buffer_size() + 1;

    // 一个map要管理几个节点。最少8个，最多是“所需节点数加2”
    // （前后各预备一个，扩充时可用）
    map_size = max(initial_map_size(), num_nodes + 2);
    map = map_allocator::allocate(map_size);
    // 以上配置出一个“具有map_size个节点”的map

    // 以下令nstart和nfinish指向map所拥有之全部节点的最中央区段
    // 保持在最中央，可使头尾两端的扩充能量一样大。每个节点可对应一个缓冲区
    map_pointer nstart = map + (map_size - num_nodes) / 2;
    map_pointer nfinish = nstart + num_nodes - 1;

    map_pointer cur;
    __STL_TRY {
        // 为map内的每个现用节点配置缓冲区。所有缓冲区加起来就是deque的
        // 可用空间（最后一个缓冲区可能留有一些余裕）
        for (cur = nstart; cur <= nfinish; ++cur)
            *cur = allocate_node();
    }
    catch(...) {
        // "commit or rollback" 语意：若非全部成功，就一个都不留
        ...
    }

    // 为deque内的两个迭代器start和end设定正确内容
    start.set_node(nstart);
    finish.set_node(nfinish);
    start.cur = start.first;        // first，cur都是public
    finish.cur = finish.first + num_elements % buffer_size();
    // 前面说过，如果刚好整除，会多配一个节点
    // 此时即令cur指向这多配的一个节点（所对应之缓冲区）的起始处
}

template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::push_back_aux(const value_type& t) {
    value_type t_copy = t;
    reserve_map_at_back();      // 若符合某种条件则必须重换一个map
    *(finish.node + 1) = allocate_node();   // 配置一个新节点（缓冲区）
    __STL_TRY {
        construct(finish.cur, t_copy);      // 针对标的元素设值
        finish.set_node(finish.node + 1);   // 改变finish，令其指向新节点
        finish.cur = finish.first;          // 设定finish的状态
    }
    __STL_UNWIND(deallocate_node(*(finish.node + 1)));
}

template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::push_front_aux(const value_type& t) {
    value_type t_copy = t;
    reserve_map_at_front();     // 若符合某种条件则必须重换一个map
    *(start.node - 1) = allocate_node();    // 配置一个新节点（缓冲区）
    __STL_TRY {
        start.set_node(start.node - 1);     // 改变start，令其指向新节点
        start.cur = start.last - 1;         // 设定start的状态
        construct(start.cur, t_copy);       // 针对标的元素设值
    }
    catch(...) {
        // "commit or rollback" 语意：若非全部成功，就一个都不留
        start.set_node(start.node + 1);
        start.cur = start.first;
        deallocate_node(*(start.node - 1));
        throw;
    }
}

template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::reallocate_map(size_type nodes_to_add, bool add_at_front) {
    size_type old_num_nodes = finish.node - start.node + 1;
    size_type new_num_nodes = old_num_nodes + nodes_to_add;

    map_pointer new_nstart;
    if (map_size > 2 * new_num_nodes) {
        new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
        if (new_nstart < start.node)
            copy(start.node, finish.node + 1, new_nstart);
        else
            copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
    } else {
        size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
        // 配置一块空间，准备给新map使用
        map_pointer new_map = map_allocator::allocate(new_map_size);
        new_nstart = new_map + (new_map_size - new_num_nodes) / 2 + (add_at_front ? nodes_to_add : 0);
        // 把原map内容拷贝过来
        copy(start.node, finish.node + 1, new_nstart);
        // 释放原map
        map_allocator::deallocate(map, map_size);
        // 设定新map的起始地址与大小
        map = new_map;
        map_size = new_map_size;
    }

    // 重新设定迭代器start和finish
    start.set_node(new_nstart);
    finish.set_node(new_nstart + old_num_nodes - 1);
}

// 只有当finish.cur == finish.first时才会被调用
template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::pop_back_aux() {
    deallocate_node(finish.first);      // 释放最后一个缓冲区
    finish.set_node(finish.node - 1);   // 调整finish的状态，使指向
    finish.cur = finish.last - 1;       // 上一个缓冲区的最后一个元素
    destroy(finish.cur);                // 将该元素析构
}

// 只有当start.cur == start.last - 1时才会被调用
template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::pop_front_aux() {
    destroy(start.cur);             // 将第一缓冲区的第一个（也是最后一个、唯一一个）元素析构

    deallocate_node(start.first);   // 释放第一缓冲区
    start.set_node(start.node + 1); // 调整start的状态，使指向
    start.cur = start.first;        // 下一个缓冲区的第一个元素
}

// 注意，最终需要保留一个缓冲区。这是deque的策略，也是deque的初始状态
template <class T, class Alloc, size_t BufSiz>
void deque<T, Alloc, BufSiz>::clear() {
    // 以下针对头尾以外的每一个缓冲区（它们一定都是饱满的）
    for (map_pointer node = start.node + 1; node < finish.node; ++node) {
        // 将缓冲区内的所有元素析构。注意，调用的是destroy第二版本
        destroy(*node, *node + buffer_size());
        // 释放缓冲区内存
        data_allocator::deallocate(*node, buffer_size());
    }

    if (start.node != finish.node) {    // 至少有头尾两个缓冲区
        destroy(start.cur, start.last); // 将头缓冲区的目前所有元素析构
        destroy(finish.first, finish.cur);  // 将尾缓冲区的目前所有元素析构
        // 释放尾缓冲区。注意，头缓冲区保留
        data_allocator::deallocate(finish.first, buffer_size());
    } else    // 只有一个缓冲区
        destroy(start.cur, finish.cur); // 将此唯一缓冲区内的所有元素析构
        // 注意，并不释放缓冲区空间。这唯一的缓冲区将保留
    
    finish = start; // 调整状态
}

template <class T, class Alloc, size_t BufSiz>
deque<T, Alloc, BufSiz>::iterator
deque<T, Alloc, BufSiz>::erase(iterator first, iterator last) {
    if (first == start && last == finish) { // 如果清除空间就是整个deque
        clear();                            // 直接调用clear()即可
        return finish;
    }
    else {
        difference_type n = last - first;               // 清除区间的长度
        difference_type elems_before = first 0 start;   // 清除区间前方的元素个数
        if (elems_before < (size() - n) / 2) {      // 如果前方的元素比较少，
            copy_forward(start, first, last);       // 向后移动前方元素（覆盖清除区间）
            iterator new_start = start + n;         // 标记deque的新起点
            destroy(start, new_start);              // 移动完毕，将冗余的元素析构
            // 以下将冗余的缓冲区释放
            for (map_pointer cur = start.node; cur < new_start.node; ++cur)
                data_allocator::deallocate(*cur, buffer_size());
            start = new_start;      // 设定deque的新起点
        }
        else {      // 如果清除区间后方的元素比较少
            copy(last, finish, first);          // 向前移动后方元素（覆盖清除区间）
            iterator new_finish = finish - n;   // 标记deque的新尾点
            destroy(new_finish, finish);        // 移动完毕，将冗余的元素析构
            // 将冗余的缓冲区释放
            for (map_pointer cur = new_finish.node + 1, cur <= finish.node; ++cur)
                data_allocator::deallocate(*cur, buffer_size());
            finish = new_finish;    // 设定deque的新尾点
        }
        return start + elems_before;
    }
}

template <class T, class Alloc, size_t BufSiz>
typename deque<T, Alloc, BufSiz>::iterator
deque<T, Alloc, BufSiz>::insert_aux(iterator pos, const value_type& x) {
    difference_type index = pos - start;    // 插入点之前的元素个数
    value_type x_copy = x;
    if (index < size() / 2) {       // 如果插入点之前的元素个数比较少
        push_front(front());        // 在最前端加入与第一元素同值的元素
        iterator front1 = start;    // 以下标示记号，然后进行元素移动
        ++front1;
        iterator front2 = front1;
        ++front2;
        pos = start + index;
        iterator pos1 = pos;
        ++pos1;
        copy(front2, pos1, front1); // 元素移动
    } else {                        // 插入点之后的元素个数比较少
        push_back(back());          // 在最尾端加入与最后元素同值的元素
        iterator back1 = finish;    // 以下标示记号，然后进行元素移动
        --back1;
        iterator back2 = back1;
        --back2;
        pos = start + index;
        copy_backward(pos, back2, back1);   // 元素移动
    }
    *pos = x_copy;  // 在插入点上设定新值
    return pos;
}
```
