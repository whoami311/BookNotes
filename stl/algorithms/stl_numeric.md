# stl_numeric

```c++
template <class InputIterator, class T>
T accumulate(InputIterator first, InputIterator last, T init) {
    for (; first != last; ++first)
        init = init + *first;   // 将每个元素值累加到初值init身上
    return init;
}

template <class InputIterator, class T, class BinaryOperation>
T accumulate(InputIterator first, InputIterator last, T init, BinaryOperation binary_op) {
    for (; first != last; ++first)
        init = binary_op(init, *first); // 对每一个元素执行二元操作
    return init;
}

template <class InputIterator, class OutputIterator>
OutputIterator adjacent_difference(InputIterator first, InputIterator last, OutputIterator result) {
    if (first == last) return result;
    *result = *first;   // 首先记录第一个元素
    return __adjacent_difference(first, last, result, value_type(first));

    // 侯捷认为（并经实证），不需像上行那样传递调用，可改用以下写法（整个函数）：
    // if (first == last) return result;
    // *result = *first;
    // iterator_traits<InputIterator>::value_type value = *first;
    // while (++first != last) {    // 走过整个区间
    //     ...以下同 __adjacent_difference() 的对应内容
    //
    // 这样的观念和做法，适用于本文件所有函数。
}

template <class InputIterator, class OutputIterator, class T>
OutputIterator __adjacent_difference(InputIterator first, InputIterator last, OutputIterator result, T*) {
    T value = *first;
    while (++first != last) {   // 走过整个区间
        T tmp = *first;
        *++result = tmp - value;    // 将相邻两元素的差额（后－前），赋值给目的端
        value = tmp;
    }
    return ++result;
}

template <class InputIterator, class OutputIterator, class BinaryOperation>
OutputIterator adjacent_difference(InputIterator first, InputIterator last, OutputIterator result, BinaryOperation binary_op) {
    if (first == last) return result;
    *result = *first;   // 首先记录第一个元素
    return __adjacent_difference(first, last, result, value_type(first), binary_op);
}

template <class InputIterator, class OutputIterator, class T, class BinaryOperation>
OutputIterator __adjacent_difference(InputIterator first, InputIterator last, OutputIterator result, T*, BinaryOperation binary_op) {
    T value = *first;
    while (++first != last) {   // 走过整个区间
        T tmp = *first;
        *++result = binary_op(tmp, value);    // 将相邻两元素的运算结果，赋值给目的端
        value = tmp;
    }
    return ++result;
}

// TODO

// 版本一，乘幂
template <class T, class Integer>
inline T power(T x, Integer n) {
    return power(x, n, multiplies<T>());    //  指定运算型式为乘法
}

// 版本二，幂次方。如果指定为乘法运算，则当n >= 0时返回x^n
// 注意，“MonoidOperaton”必须满足结合律（associative），
// 但不满足交换律（commutative）
template <class T, class Integer, class MonoidOperation>
T power(T x, Integer, MonoidOperaton op) {
    if (n == 0)
        return identity_element(op);    // 取出“证同元素” identity element
    else {
        while ((n & 1) == 0) {
            n >>= 1;
            x = op(x, x);
        }

        T result = x;
        n >>= 1;
        while (n != 0) {
            x = op(x, x);
            if ((n & 1) != 0)
                result = op(result, x);
            n >>= 1;
        }
        return result;
    }
}

// 函数意义：在[first, last)区间内填入value, value+1, value+2...
template <class ForwardIterator, class T>
void iota(ForwardIterator first, ForwardIterator last, T value) {
    while (first != last) *first++ = value++;
}
```
