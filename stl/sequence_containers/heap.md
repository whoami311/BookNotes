# heap

```c++
template <class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
    // 注意，此函数被调用时，新元素应已置于底部容器的最尾端
    __push_heap_aux(first, last, distance_type(first), value_type(first));
}

template <class RandomAccessIterator, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first, RandomAccessIterator last, Distance*, T*) {
    __push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)));
    // 以上系根据implicit representation heap的结构特性：新值必置于底部
    // 容器的最尾端，此即第一个洞号：（last - first) - 1
}

// 以下这组push_back()不允许指定“大小比较标准”
template <class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex, Distance topIndex, T value) {
    Distance parent = (holeIndex - 1) / 2;  // 找出父节点
    while (holeIndex > topIndex && *(first + parent) < value) {
        // 当尚未达到顶端，且父节点小于新值（于是不符合heap的次序特性）
        // 由于以上使用operator<，可知STL heap是一种max-heap（大者为父）
        *(first + holeIndex) = *(first + parent);   // 令洞值为父值
        holeIndex = parent; // percolate up：调整洞号，向上提升至父节点
        parent = (holeIndex - 1) / 2;       // 新洞的父节点
    }   // 持续至顶端，或满足heap的次序特性为止
    *(first + holeIndex) = value;   // 令洞值为新值，完成插入操作
}

template <class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __pop_heap_aux(first, last, value_type(first));
}

template <class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*) {
    __pop_heap(first, last-1, last-1, T(*(last-1)), distance_type(first));
    // 以上，根据implicit representation heap的次序特性，pop操作的结果
    // 应为底部容器的第一个元素。因此，首先设定欲调整值为尾值，然后将首值调至
    // 尾节点（所以以上将迭代器result设为last-1）。然后重整[first, last-1)，
    // 使之重新成一个合格的heap
}

// 以下这组__pop_back()不允许指定“大小比较标准”
template <class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last, RandomAccessIterator result, T value, Distance*) {
    *result = *first;   // 设定尾值为首值，于是尾值即为欲求结果，
                        // 可由客户端稍后再以底层容器之pop_back()取出尾值
    __adjust_heap(first, Distance(0), Distance(last - first), value);
    // 以上欲重新调整heap，洞号为0（亦即树根处），欲调整值为value（原尾值）
}

// 以下这个__adjust_heap()不允许指定“大小比较标准”
template <class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value) {
    Distance topIndex = holeIndex;
    Distance secondChild = 2 * holeIndex + 2;   // 洞节点之右子节点
    while (secondChild < len) {
        // 比较洞节点之左右两个子值，然后以secondChild代表较大子节点
        if (*(first + secondChild) < *(first + (secondChild - 1)))
            secondChild--;
        // Percolate down：令较大子值为洞值，再令洞号下移至较大子节点处
        *(first + holeIndex) = *(first + secondChild);
        holeIndex = secondChild;
        // 找出新洞节点的右子节点
        secondChild = 2 * (secondChild + 1);
    }
    if (secondChild == len) {   // 没有右子结点，只有左子节点
        // Percolate down：令左子值为洞值，再令洞号下移至左子节点处
        *(first + holeIndex) = *(first + (secondChild - 1));
        holeIndex = secondChild - 1;
    }
    // 此时（可能）尚未满足次序特性。执行一次percolate up操作
    // 读者回应：不可如此，试套4.7.4节范例即知。侯捷测试：验证后的确不行。
    __push_heap(first, holeIndex, topIndex, value);
}

// 以下这个sort_heap()不允许指定“大小比较标准”
template <class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
    // 以下，每执行一次pop_heap()，极值（在STL heap中为极大值）即被放在尾端。
    // 扣除尾端再执行一次pop_heap()，次极值又被放在新尾端。一直下去，最后即得
    // 排序结果
    while (last - first > 1)
        pop_heap(first, last--);    // 每执行pop_heap()一次，操作范围即退缩一格
}

// 将[first, last)排列为一个heap
template <class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
    __make_heap(first, last, value_type(first), distance_type(first));
}

// 以下这组make_heap()不允许指定“大小比较标准”
template <class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, RandomAccessIterator last, T*, Distance*) {
    if (last - first < 2) return;   // 如果长度为0或1，不必重新排列
    Distance len = last - first;
    // 找出第一个需要重排的子树头部，以parent标示出。由于任何叶节点都不需执行
    // perlocate down，所以有以下计算。parent命名不佳，名为holeIndex更好
    Distance parent = (len - 2) / 2;

    while (true) {
        // 重排以parent为首的子树。len是为了让__adjust_heap()判断操作范围
        __adjust_heap(first, parent, len, T(*(first + parent)));
        if (parent == 0) return;    // 走完根节点，就结束
        parent--;                   // （已重排之子树的）头部向前一个节点
    }
}
```
