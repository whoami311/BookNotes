# stl_algobase

```c++
template <class InputIterator1, class InputIterator2>
inline bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2) {
    // 以下，将序列一走过一遍，序列二亦步亦趋
    // 如果序列一的元素个数多过序列二的元素个数，就糟糕了
    for (; first1 != last1; ++first1, ++first2)
        if (*first1 != first2)          // 只要对应元素不相等
            return false;               // 就结束并返回false
    return true;                    // 至此，全部相等，返回true
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
inline bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, BinaryPredicate binary_pred) {
    for (; first1 != last1; ++first1, ++first2)
        if (!binary_pred(*first1, *first2))
            return false;
    return true;
}

// 将[first, last)内的所有元素改填新值
template <class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& value) {
    for (; first != last; ++first)  // 迭代走过整个区间
        *first = value;             // 设定新值
}

// 将[first, last)内的前n个元素改填新值，返回的迭代器指向被填入的最后一个元素的下一位置。
template <class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T& value) {
    for (; n>0; --n, ++first)   // 经过n个元素
        *first = value;         // 设定新值
    return first;
}

// TODO

// 取两个对象中的较大值。
template <class T>
inline const T& max(const T& a, const T& b) {
    return a < b ? b : a;
}

template <class T, class Compare>
inline const T& max(const T& a, const T& b, Compare comp) {
    return comp(a, b) ? b : a;  // 由comp决定“大小比较”标准
}

// 取两个对象中的较小值。
template <class T>
inline const T& min(const T& a, const T& b) {
    return b < a ? b : a;
}

template <class T, class Compare>
inline const T& max(const T& a, const T& b, Compare comp) {
    return comp(b, a) ? b : a;  // 由comp决定“大小比较”标准
}

// TODO

// 交换（对调）两个对象的内容
template <class T>
inline void swap(T& a, T& b) {
    T tmp = a;
    a = b;
    b = tmp;
}

// 完全泛化版本
template <class InputIterator, class OutputIterator>
inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result) {
    return __copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
}

// 特殊版本（1）。重载形式
inline char* copy(const char* first, const char* last, char* result) {
    memmove(result, first, last - first);
    return result + (last - result);
}

// 特殊版本（2）。重载形式
inline wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result) {
    memmove(result, first, sizeof(wchar_t) * (last - first));
    return result + (last - result);
}

// 完全泛化版本
template <class InputIterator, class OutputIterator>
struct __copy_dispatch {
    OutputIterator operator()(InputIterator first, InputIterator last, OutputIterator result) {
        return __copy(first, last, result, iterator_category(first));
    }
};

// 偏特化版本（1），两个参数都是T*指针形式
template <class T>
struct __copy_dispatch<T*, T*> {
    T* operator()(T* first, T* last, T* result) {
        typedef typename __type_traits<T>::has_trivial_assignment_operator t;
        return __copy_t(first, last, result, t());
    }
};

// 偏特化版本（2），第一个参数为const T*指针形式，第二个参数为T*指针形式
template <class T>
struct __copy_dispatch<const T*, T*> {
    T* operator()(const T* first, const T* last, T* result) {
        typedef typename __type_traits<T>::has_trivial_assignment_operator t;
        return __copy_t(first, last, result, t());
    }
};

// InputIterator版本
template <class InputIterator, class OutputIterator>
inline OutputIterator __copy(InputIterator first, InputIterator last, OutputIterator result, input_iterator_tag) {
    // 以迭代器等同于否，决定循环是否继续。速度慢
    for (; first != last; ++result, ++first)
        *result = *first;   // assignment operator
    return result;
}

// RandomAccessIterator版本
template <class RandomAccessIterator, class OutputIterator>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last, OutputIterator result, input_iterator_tag) {
    // 又划分出一个函数，为的是其他地方也可能用到
    return __copy_d(first, last, result, distance_type(first));
}

template <class RandomAccessIterator, class OutputIterator, class Distance>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last, OutputIterator result, Distance*) {
    // 以n决定循环的执行次数。速度快
    for (Distance n = last - first; n > 0; --n, ++result, ++first)
        *result = *first;       // assignment operator
    return result;
}

// 以下版本适用于“指针所指之对象具备trivial assignment operator”
template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __true_type) {
    memmove(result, first, sizeof(T) * (last - first));
    return result + (last - first);
}

// 以下版本适用于“指针所指之对象具备non-trivial assignment operator”
template <class T>
inline T* __copy_t(const T* first, const T* last, T* result, __false_type) {
    // 原生指针毕竟是一种RandomAccessIterator，所以交给__copy_d()完成
    return __copy_d(first, last, result, (ptrdiff_t*)0);
}
```
