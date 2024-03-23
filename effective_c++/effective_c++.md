# Effective C++

## chapter 0

被声明为 `explicit` 的构造函数通常比其 `non-explicit` 兄弟更受欢迎，因为它们禁止编译期执行非预期的类型转换。除非有一个好理由允许构造函数被用于隐式类型转换，否则我会把它声明为 `explicit`。

## chapter 1

1. 