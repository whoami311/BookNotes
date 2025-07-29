# Effective C++

## chapter 0

被声明为 `explicit` 的构造函数通常比其 non-`explicit` 兄弟更受欢迎，因为它们禁止编译期执行非预期的类型转换。除非有一个好理由允许构造函数被用于隐式类型转换，否则我会把它声明为 `explicit`。

## chapter 1 让自己习惯 C++

### item 01：视 C++ 为一个语言联邦

C++ 是个多重范型编程语言，一个同时支持面向过程形式（procedural）、面向对象形式（object-oriented）、函数形式（functional）、泛型形式（generic）、元编程形式（metaprogramming）的语言。

### item 02：尽量以 const, enum, inline 替换 #define

- 对于单纯变量，最好以 `const` 对象或 `enum` 替换 `#define`。
- 对于形似函数的宏（macros），最好改用 `inline` 函数替换 `#define`。

### item 03：尽可能使用 const

- 将某些东西声明为 `const` 可帮助编译器侦测出错误用法。`const` 可被施加于任何作用域内的对象、函数参数、函数返回类型、成员函数本体。
- 编译器强制实施 bitwise constness，但你编写程序时应该使用“概念上的常量性”（conceptual constness）。
- 当 const 和 non-`const` 成员函数有着实质等价的实现时，令 non-`const` 版本调用 const 版本可避免代码重复。

### item 04：确定对象被使用前已先被初始化

- 为内置型对象进行手动初始化，因为 C++ 不保证初始化它们。
- 构造函数最好使用成员初值列（member initialization list），而不要在构造函数本体内使用赋值操作（assignment）。初值列列出的成员变量，其排列次序应该和它们在 class 中的声明次序相同。
- 为免除“跨编译单元之初始化次序”问题，请以 local static 对象替换 non-local-static 对象。
