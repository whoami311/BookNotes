# stl_algo

```c++
// 运用equality操作符，将[first, last)区间内的每一个元素拿来和指定值value比较，并返回与value相等的元素个数。
template <class InputIterator, class T>
typename iterator_traits<InputIterator>::difference_type count(InputIterator first, InputIterator last, const T& value) {
    // 以下声明一个计数器n
    typename iterator_traits<InputIterator>::difference_type n = 0;
    for (; first != last; ++first)  // 整个区间走一遍
        if (*first == value)        // 如果元素值和value相等
            ++n;                    // 计数器累加1
    return n;
}

// 根据equality操作符，循序查找[first, last)内的所有元素，找出第一个匹配“等同（equality）条件”者。如果找到，就返回一个InputIterator指向该元素，否则返回迭代器last。
template <class InputIterator, class T>
InputIterator find(InputIterator first, InputIterator last, const T& value) {
    while (first != last && *first != value)
        ++first;
    return first;
}

// 以[first, last)区间内的某些元素作为查找目标，寻找它们在[first, last)区间内的第一次出现地点。
// 版本一
template <class InputIterator, class ForwardIterator>
InputIterator find_first_of(InputIterator first1, InputIterator last1, ForwardIterator first2, ForwardIterator last2) {
    for (; first1 != last1; ++first1)   // 遍访序列一
        // 以下，根据序列二的每个元素
        for (ForwardIterator iter = first2; iter != last2; ++iter)
            if (*first1 == *iter)   // 如果序列一的元素等于序列二的元素
                return first1;      // 找到了，结束
    return last1;
}

// 版本二
template <class InputIterator, class ForwardIterator, class BinaryPredicate>
InputIterator find_first_of(InputIterator first1, InputIterator last1, ForwardIterator first2, ForwardIterator last2, BinaryPredicate comp) {
    for (; first1 != last1; ++first1)   // 遍访序列一
        // 以下，根据序列二的每个元素
        for (ForwardIterator iter = first2; iter != last2; ++iter)
            if (comp(*first1, *iter))   // 如果序列一和序列二的元素满足comp条件
                return first1;      // 找到了，结束
    return last1;
}

// 将仿函数f施行于[first, last)区间内的每一个元素身上。f不可以改变元素内容，因为first和last都是InputIterators，不保证接受赋值行为（assignment）。
template <class InputIterator, class Function>
Function for_each(InputIterator first, InputIterator last, Function f) {
    for (; first != last; ++first)
        f(*first);      // 调用仿函数f的function call操作符。返回值被忽略
    return f;
}

// 将仿函数gen的运算结果填写在[first, last)区间内的所有元素身上。所谓填写，用的是迭代器所指元素之assignment操作符。
template <class ForwardIterator, class Generator>
void generate(ForwardIterator first, ForwardIterator last, Generator gen) {
    for (; first != last; ++first)  // 整个序列区间
        *first = gen();
}

// 判断序列二S2是否“涵盖于”序列一S1。S1和S2都必须是有序集合，其中的元素都可重复（不必唯一）。
// 版本一。判断区间二的每个元素值是否都存在于区间一
// 前提：区间一和区间二都是sorted ranges
template <class InputIterator1, class InputIterator2>
bool includes(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1 && first2 != last2)  // 两个区间都尚未走完
        if (*first2 < *first1)      // 序列二的元素小于序列一的元素
            return false;           // “涵盖”的情况必然不成立。结束执行
        else if (*first1 < *first2) // 序列二的元素大于序列一的元素
            ++first1;               // 序列一前进1
        else                        // *first1 == *first2
            ++first1, ++first2;     // 两序列各自前进1
    
    return first2 == last2;     // 有一个序列走完了，判断最后一关
}

// 版本二。判断序列一内是否有个子序列，其与序列二的每个对应元素都满足二元运算comp
// 前提：序列一和序列二都是sorted ranges
template <class InputIterator1, class InputIterator2, class Compare>
bool includes(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, Compare comp) {
    while (first1 != last1 && first2 != last2)  // 两个区间都尚未走完
        if (comp(*first2, *first1))         // 序列二的元素小于序列一的元素
            return false;                   // “涵盖”的情况必然不成立。结束执行
        else if (comp(*first1, *first2))    // 序列二的元素大于序列一的元素
            ++first1;                       // 序列一前进1
        else                                // *first1 == *first2
            ++first1, ++first2;             // 两序列各自前进1
    
    return first2 == last2;     // 有一个序列走完了，判断最后一关
}

// 返回一个迭代器，指向序列之中数值最大的元素
// 版本一
template <class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last) {
    if (frist == last)
        return first;
    ForwardIterator result = first;
    while (++first != last)
        if (*result < *first)
            result = first;         // 如果目前元素比较大，就登记起来
    return result;
}

// 版本二
template <class ForwardIterator, class Compare>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last, Compare comp) {
    if (frist == last)
        return first;
    ForwardIterator result = first;
    while (++first != last)
        if (comp(*result, *first))
            result = first;
    return result;
}

// 将两个经过排序的结合S1和S2，合并起来置于另一段空间。所得结果也是一个有序（sorted）序列。返回一个迭代器，指向最后结果序列的最后一个元素的下一位置。
// 版本一
template <class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, OutputIterator result) {
    while (first1 != last1 && first2 != last2) {    // 两个序列都尚未走完
        if (*first2 < *first1) {    // 序列二的元素比较小
            *result = *first2;      // 登记序列二的元素
            ++first2;               // 序列二前进1
        } else {                    // 序列二的元素不比较小
            *result = *first1;      // 登记序列一的元素
            ++first1;               // 序列一前进1
        }
        ++result;
    }
    // 最后剩余元素以copy复制到目的端。以下两个序列一定至少有一个为空
    return copy(first2, last2, copy(first1, last1, result));
}

// 版本二
template <class InputIterator1, class InputIterator2, class OutputIterator, class Compare>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2, OutputIterator result, Compare comp) {
    while (first1 != last1 && first2 != last2) {    // 两个序列都尚未走完
        if (comp(*first2, *first1)) {   // 比较两序列的元素
            *result = *first2;      // 登记序列二的元素
            ++first2;               // 序列二前进1
        } else {
            *result = *first1;      // 登记序列一的元素
            ++first1;               // 序列一前进1
        }
        ++result;
    }
    // 最后剩余元素以copy复制到目的端。以下两个序列一定至少有一个为空
    return copy(first2, last2, copy(first1, last1, result));
}

// 返回一个迭代器，指向序列之中数值最小的元素
// 版本一
template <class ForwardIterator>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last) {
    if (frist == last)
        return first;
    ForwardIterator result = first;
    while (++first != last)
        if (*first < *result)
            result = first;         // 如果目前元素比较小，就登记起来
    return result;
}

// 版本二
template <class ForwardIterator, class Compare>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last, Compare comp) {
    if (frist == last)
        return first;
    ForwardIterator result = first;
    while (++first != last)
        if (comp(*first, *result))
            result = first;
    return result;
}

// 将[first, last)区间内的所有old_value都已new_value取代。
template <class ForwardIterator, class T>
void replace(ForwardIterator first, ForwardIterator last, const T& old_value, const T& new_value) {
    // 将区间内的所有old_value都已new_value取代
    for (; first != last; ++first)
        if (*first == old_value)
            *first = new_value;
}

// 将序列[first, last)的元素在原容器中颠倒重排。
// 分派函数（dispatch function）
template <class BidirectionalIterator>
inline void reverse(BidirectionalIterator first, BidirectionalIterator last) {
    __reverse(first, last, iterator_category(first));
}

// reverse的random access iterator版
template <class RandomAccessIterator>
void __reverse(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag) {
    // 以下，头尾两两互换，然后头部累进一个位置，尾部累退一个位置。两者交错时即停止
    // 注意，只有random iterators才能做以下的first < last判断
    while (first < last)
        iter_swap(first++, --last);
}

// 在序列一[first1, last1)所涵盖的区间中，查找序列二[first2, last2)的首次出现点。如果序列一内不存在与序列二完全匹配的子序列，便返回迭代器last1。
// 版本一
template <class ForwardIterator1, class ForwardIterator2>
inline ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2) {
    return __search(first1, last1, first2, last2, distance_type(first1), distance_type(first2));
}

template <class ForwardIterator1, class ForwardIterator2, class Distance1, class Distance2>
ForwardIterator1 __search(ForwardIterator1 first1, ForwardIterator1 last1, ForwardIterator2 first2, ForwardIterator2 last2, Distance1*, Distance2*) {
    Distance1 d1 = 0;
    distance(first1, last1, d1);
    Distance1 d2 = 0;
    distance(first2, last2, d2);
    if (d1 < d2)
        return last1;   //  如果第二序列大于第一序列，不可能成为其子序列

    ForwardIterator1 current1 = first1;
    ForwardIterator2 current2 = first2;

    while (current2 != last2)   // 遍历整个第二序列
        if (*current1 == *current2) {   // 如果这个元素相同
            ++current1;                 // 调整，以便比对下一个元素
            ++current2;
        } else {                        // 如果这个元素不等
            if (d1 == d2)               // 如果两序列一样长
                return last1;           // 表示不可能成功了
            else {                      // 两序列不一样长（至此肯定是序列一大于序列二）
                current1 = ++first1;    // 调整第一序列的标兵，
                current2 = first2;      // 准备在新起点上再找一次
                --d1;                   // 已经排除了序列一的一个元素，所以序列一的长度要减1
            }
        }
    return first1;
}

// 第一版本以仿函数op作用于[first, last)中的每一个元素身上，并以其结果产生出一个新序列。第二版本以仿函数binary_op作用于一双元素身上（其中一个元素来自[first1, last)，另一个元素来自“从first2开始的序列”），并以其结果产生出一个新序列。如果第二序列的元素少于第一序列，执行结果未可预期。
// 版本一
template <class InputIterator1, class OutputIterator, class UnaryOperation>
OutputIterator transform(InputIterator first, InputIterator last, OutputIterator result, UnaryOperation op) {
    for (; first != last; ++first, ++result)
        *result = op(*first);
    return result;
}

// 版本二
template <class InputIterator1, class InputIterator2, class OutputIterator, class BinaryOperation>
OutputIterator transform(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, OutputIterator result, BinaryOperation binary_op) {
    for (; first1 != last1; ++first1, ++first2, ++result)
        *result = binary(*first1, *first2);
    return result;
}

// 返回一个迭代器，指向第一个“不小于value”的元素。
// 版本一
template <class ForwardIterator, class T>
inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& value) {
    return __lower_bound(first, last, value, distance_type(first), iterator_category(first));
}

// 版本二
template <class ForwardIterator, class T, class Compare>
inline ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last, const T& value, Compare comp) {
    return __lower_bound(first, last, value, comp, distance_type(first), iterator_category(first));
}

// 版本一的forward_iterator版本
template <class ForwardIterator, class T, class Distance>
ForwardIterator __lower_bound(ForwardIterator first, ForwardIterator last, const T& value, Distance*, forward_iterator_tag) {
    Distance len = 0;
    distance(first, last, len); // 求取整个区间的长度len
    Distance half;
    ForwardIterator middle;

    while (len > 0) {
        half = len >> 1;            // 除以2
        middle = first;             // 这两行令middle指向中间位置
        advance(middle, half);
        if (*middle < value) {      // 如果中间位置的元素值 < 标的值
            first = middle;         // 这两行令first指向middle的下一位置
            ++first;
            len = len - half - 1;   // 修正len，回头测试循环的结束条件
        } else
            len = half;             // 修正len，回头测试循环的结束条件
    }
    return first;
}

// 版本一的random_access_iterator版本
template <class RandomAccessIterator, class T, class Distance>
RandomAccessIterator __lower_bound(RandomAccessIterator first, RandomAccessIterator last, const T& value, Distance*, random_access_iterator_tag) {
    Distance len = last - first;    // 求取整个区间的长度len
    Distance half;
    RandomAccessIterator middle;

    while (len > 0) {
        half = len >> 1;            // 除以2
        middle = first + half;      // 令middle指向中间位置（r - a - i才能如此）
        if (*middle < value) {      // 如果中间位置的元素值 < 标的值
            first = middle + 1;     // 令first指向middle的下一位置
            len = len - half - 1;   // 修正len，回头测试循环的结束条件
        } else
            len = half;             // 修正len，回头测试循环的结束条件
    }
    return first;
}

// 返回“在不破坏顺序的情况下，可插入value的最后一个合适位置”。
// 版本一
template <class ForwardIterator, class T>
inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& value) {
    return __upper_bound(first, last, value, distance_type(first), iterator_category(first));
}

// 版本二
template <class ForwardIterator, class T, class Compare>
inline ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last, const T& value, Compare comp) {
    return __upper_bound(first, last, value, comp, distance_type(first), iterator_category(first));
}

// 版本一的forward_iterator版本
template <class ForwardIterator, class T, class Distance>
ForwardIterator __upper_bound(ForwardIterator first, ForwardIterator last, const T& value, Distance*, forward_iterator_tag) {
    Distance len = 0;
    distance(first, last, len); // 求取整个区间的长度len
    Distance half;
    ForwardIterator middle;

    while (len > 0) {
        half = len >> 1;            // 除以2
        middle = first;             // 这两行令middle指向中间位置
        advance(middle, half);
        if (value < *middle) {      // 如果中间位置的元素值 > 标的值
            len = half;             // 修正len，回头测试循环的结束条件
        else {
            first = middle;         // 这两行令first指向middle的下一位置
            ++first;
            len = len - half - 1;   // 修正len，回头测试循环的结束条件
        }
    }
    return first;
}

// 版本一的random_access_iterator版本
template <class RandomAccessIterator, class T, class Distance>
RandomAccessIterator __upper_bound(RandomAccessIterator first, RandomAccessIterator last, const T& value, Distance*, random_access_iterator_tag) {
    Distance len = last - first;    // 求取整个区间的长度len
    Distance half;
    RandomAccessIterator middle;

    while (len > 0) {
        half = len >> 1;            // 除以2
        middle = first + half;      // 令middle指向中间位置（r - a - i才能如此）
        if (value < *middle)        // 如果中间位置的元素值 > 标的值
            len = half;             // 修正len，回头测试循环的结束条件
        else {
            first = middle + 1;     // 令first指向middle的下一位置
            len = len - half - 1;   // 修正len，回头测试循环的结束条件
        }
            
    }
    return first;
}

// sort

// 版本一
template <class RandomAccessIterator>
void __insertion_sort(RandomAccessIterator first, RandomAccessIterator last) {
    if (first == last)
        return;
    for (RandomAccessIterator i = first + 1; i != last; ++i) {
        __linear_insert(first, i, value_type(first));
        // 以上，[first, i)形成一个子区间
    }
}

// 版本一辅助函数
template <class RandomAccessIterator, class T>
inline void __linear_insert(RandomAccessIterator first, RandomAccessIterator last, T*) {
    T value = *last;    // 记录尾元素
    if (value < *first) {   // 尾比头还小（注意，头端必为最小元素）
        // 那么就别一个个比较了，一次做完爽快些...
        copy_backward(first, last, last + 1);   // 将整个区间向右递移一个位置
        *first = value;     // 令头元素等于原先的尾元素值
    } else  // 尾不小于头
        __unguarded_linear_insert(last, value);
}

// 版本一辅助函数
template <class RandomAccessIterator, class T>
void __unguarded_linear_insert(RandomAccessIterator last, T value) {
    RandomAccessIterator next = last;
    --next;
    // insertion sort的内循环
    // 注意，一旦不再出现逆转对（inversion），循环就可以结束了
    while (value < *next) { // 逆转对（inversion存在）
        *last = *next;      // 调整
        last = next;        // 调整迭代器
        --next;             // 左移一个位置
    }
    *last = value;          // value的正确落脚处
}

// 返回a,b,c之居中者
template <class T>
inline const T& __median(const T& a, const T& b, const T& c) {
    if (a < b)
        if (b < c)      // a < b < c
            return b;
        else if (a < c) // a < b, b >= c, a < c
            return c;
        else
            return a;
    else if (a < c)     // c > a >= b
        return a;
    else if (b < c)     // a >= b, a >= c, b < c
        return c;
    else
        return b;
}

// 版本一
template <class RandomAccessIterator, class T>
RandomAccessIterator __unguarded_partition(RandomAccessIterator first, RandomAccessIterator last, T pivot) {
    while (true) {
        while (*first < pivot)
            ++first;    // first找到>=pivot的元素就停下来
        --last;         // 调整
        while (pivot < *last)
            --last;     // last找到<=pivot的元素就停下来
        // 注意，以下first < last判断操作，只适用于random iterator
        if (!(first < last))
            return first;   // 交错，结束循环
        iter_swap(first, last); // 大小值交换
        ++first;                // 调整
    }
}

// 版本一
// 千万注意：sort()只适用于RandomAccessIterator
template <class RandomAccessIterator>
inline void sort(RandomAccessIterator first, RandomAccessIterator last) {
    if (first != last) {
        __introsort_loop(first, last, value_type(first), __lg(last - first) * 2);
        __final_insertion_sort(first, last);
    }
}

// 找出 2^k <= n 的最大值k。
template <class Size>
inline Size __lg(Size n) {
    Size k;
    for (k=0; n>1; n >>= 1)
        ++k;
    return k;
}

// 版本一
// 注意，本函数内的许多迭代器运算操作，都只适用于RandomAccess Iterators
template <class RandomAccessIterator, class T, class Size>
void __introsort_loop(RandomAccessIterator first, RandomAccessIterator last, T*, Size depth_limit) {
    // 以下，__stl_threshold是个全局函数，稍早定义为const int 16
    while (last - first > __stl_threshold) {
        if (depth_limit == 0) {                 // 至此，分割恶化
            partial_sort(first, last, last);    // 改用heapsort
            return;
        }
        --depth_limit;
        // 以下是median-of-3 partition，选择一个够好的枢轴并决定分割点
        // 分割点将落在迭代器cut身上
        RandomAccessIterator cut = __unguarded_partition(first, last, T(__median(*first, *(first + (last - first) / 2), *(last - 1))));
        // 对右半段递归进行sort.
        __introsort_loop(cut, last, value_type(first), depth_limit);
        last = cut;
        // 现在回到while循环，准备对左半段递归进行sort
        // 这种写法可读性较差，效率并没有比较好
    }
}

const int __stl_threshold = 16;

// 版本一
template <class RandomAccessIterator>
void __final_insertion_sort(RandomAccessIterator first, RandomAccessIterator last) {
    if (last - first > __stl_threshold) {
        __insertion_sort(first, first + __stl_threshold);
        __unguarded_insertion_sort(first + __stl_threshold, last);
    } else
        __insertion_sort(first, last);
}

// 版本一
template <class RandomAccessIterator>
inline void __unguarded_insertion_sort(RandomAccessIterator first, RandomAccessIterator last) {
    __unguarded_insertion_sort_aux(first, last, value_type(first));
}

// 版本一
template <class RandomAccessIterator, class T>
void __unguarded_insertion_sort_aux(RandomAccessIterator first, RandomAccessIterator last, T*) {
    for (RandomAccessIterator i = first; i != last; ++i)
        __unguarded_linear_insert(i, T(*i));
}
```
