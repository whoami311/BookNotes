# 特化与重载

## 当“泛型代码”不是特别适用的时候

### 透明自定义

C++ 模板提供了透明自定义函数模板和类模板的一些方法。对于函数模板的透明自定义，可以通过重载机制来实现。

## 重载函数模板

### 签名

如果两个函数有不同的签名，那么它们可以在一个程序中同时存在。我们对函数签名的定义如下。

- 函数的非受限名称（或产生自函数模板的这类名称）。
- 该名称的类作用域或命名空间作用域。如果该函数名称具有内部链接，则作用域为该名称声明所在的编译单元。
- 函数的 `const`、`volatile` 或 `const volatile` 限定符（如果它是一个具有这类限定符的成员函数）。
- 函数的 `&` 或 `&&` 限定符（如果它是一个具有这类限定符的成员函数）。
- 函数参数的类型（如果这个函数是由函数模板生成的，那么指的是模板参数被替换之前的类型）。
- 如果这个函数是由函数模板生成的，那么包括它的返回类型。
- 如果这个函数是由函数模板生成的，那么包括它的模板参数和模板实参。

### 正式的排序规则

重载解析规则明确如下内容。

- 默认实参包含的函数调用参数和未使用的省略号参数，在后面的内容中将不再考虑。
- 然后，我们通过替换每个模板参数来虚构两个不同的实参类型（或转换函数模板的返回类型）列表，如下所示。
    - 用唯一的类型替换每个模板类型参数。
    - 用唯一的类模板替换每个模板参数。
    - 用适当类型的唯一值替换每个非类型模板参数（在此上下文中创建的类型、模板和值，不同于程序员在其他上下文中使用的或编译器在其他上下文中合成的任何其他类型、模板和值）。
- 如果第 2 个模板可以成功对第 1 个综合参数类型列表进行模板实参演绎，并且能够精确匹配，但反之则不然，则第 1 个模板比第 2 个模板更加特殊。与此相反，如果第 1 个模板可以成功对第 2 个综合参数类型列表进行模板实参演绎，并且能够精确匹配，但反之则不然，则第 2 个模板比第 1 个模板更加特殊。否则的话（要么都没有演绎成功，要么两者都演绎成功），我们认为这两个模板之间没有排序关系。

### 模板和非模板

函数模板可以和非模板函数重载。在其他条件相同的时候，实际的函数调用将会优先选择非模板函数。

当成员函数模板可能隐藏拷贝或移动构造函数时，通常必须部分禁用它们。

### 变参函数模板

当正式的排序规则应用于变参模板时，每个模板参数包都被一个单独的组合类型、类模板或值所取代。

## 显式特化

类模板和变量模板是不能被重载的。因此，我们选择了另一种机制来实现类模板的透明自定义：显式特化。C++ 标准中显式特化是指一种我们也称之为全局特化的语言特性。它为模板提供了一个实现，模板参数被完全替换：没有模板参数被保留。类模板、函数模板和变量模板可以完全专用化。

### 全局的类模板特化

全局特化是由 3 个标记——`template`、`<` 和 `>` 组成的序列。此外，类名称声明后面的内容是要进行特化的模板实参。

全局特化的实现不需要和泛型定义有任何关联：这允许我们拥有不同名称的成员函数。全局特化完全由类模板的名称决定。

指定的模板实参列表必须和模板参数列表一一对应。例如，为模板类型参数指定非类型参数是非法的。但是，如果模板参数具有默认模板实参，那么模板实参就是可选的。

（模板）全局特化的声明并不一定必须是定义。但是，当全局特化声明后，泛型定义将不再用于给定的模板实参集。因此，如果需要该特化的定义，但在这之前并没有提供这个定义，那么程序将出错。对于类模板特化来说，“前置声明”类型有时会很有用，因为这样就可以构造相互依赖的类型。全局特化声明与普通类声明是类似的（它并不是模板声明），唯一的区别在于语法和该特化的声明必须匹配前面的模板声明。因为全局特化声明并不是模板声明，所以可以使用普通的类外成员定义语法，来定义全局类模板特化的成员（换句话说，不能指定 `template<>` 前缀）。

全局特化是对泛型模板某个实例化体的替换。如果程序中同时存在模板的显式版本和生成的版本，那么该程序将是无效的。

必须注意确保特化的声明对泛型模板的所有用户都是可见的。实际上，这意味着：特化的声明通常应该在其头文件中的模板声明之后。当泛型实现来自外部资源时（因此不应修改相应的头文件），这种做法不一定很适用，但我们可以创建一个包含泛型模板的头文件，然后对该特化进行声明，以避免这些难以发现的错误，这可能是很有必要的。我们发现，一般来说，最好避免外部资源的模板的特化，除非它明确是为此目的而设计的。
