# 类型属性重载

## 算法特化

引入更特殊的泛型算法变体的设计和优化方法被称为算法特化。更特殊的变体应用于泛型算法的有效输入的子集，基于类型的特定类型或属性来识别该子集，并且通常比该泛型算法的最常用实现更为有效。

实现算法特化的关键是：当更特殊的变体适用时，算法会自动选择，而调用方不必注意这些变体是存在的。

并不是所有特化程度更高的算法变体都可以直接转换为函数模板，这些函数模板提供了正确的局部排序实现。

## 标签调度

算法特化的一种方法是：使用标识变体的唯一类型来“标识”算法的不同实现变体。

在 C++ 标准库中，可用的标签定义如下（它们使用继承关系来表达某个标签类型是从另一个类别派生而来的）：

```c++
namespace std {
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : public input_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};
}
```

有效利用标签调度的关键在于标签之间的关系。

当算法使用的属性有一个自然的层次结构和一组提供这些标签的现有特征时，标签调度可以很好地工作。当算法特化依赖于特定类型属性时，就不那么方便了，例如类型 `T` 是否有一个普通的拷贝赋值运算符。为此，我们需要一种更强大的技术。

## 启用 / 禁用函数模板

算法特化涉及提供基于模板实参属性选择的不同函数模板。遗憾的是，函数模板的局部排序和重载解析都不足以表示更高级形式的算法特化。

C++ 标准库提供的一个 helper 类是 `std::enable_if`，本节讨论如何通过引入相应的别名模板来实现这个 helper 类。

```c++
// typeoverlaod/enableif.hpp
template <bool, typename T = void>
struct EnableIfT {};

template <typename T>
struct EnableIfT<true, T> {
    using Type = T;
};

template <bool Cond, typename T = void>
using EnableIf = typename EnableIfT<Cond, T>::Type;
```

由于 `EnableIf` 扩展到一个类型，因此实现为一个别名模板。我们希望使用偏特化来实现它，但是别名模板不能偏特化。幸运的是，我们可以引入一个 helper 类模板 `EnableIfT`，它完成了我们需要的实际工作，如果只需从 helper 模板中选择结果类型，就可以启用别名模板。当条件为 `true` 时，`EnableIfT<...>::Type`（因此 `EnableIf<...>`）只计算第 2 个模板实参 `T`。当条件为 `false` 时，`EnableIf` 不会生成有效的类型，因为 `EnableIfT` 的主类模板没有名为 `Type` 的成员。通常，这会是一个错误，但在 SFINAE 上下文（如函数模板的返回类型）中，它会导致模板实参演绎失败，从而将函数模板从考虑范围中删除。

我们现在已经确定了：如何显式地“激活”应用于类型的特化较好的模板。但是，这还不够：我们还必须“禁用”特化不理想的模板，因为编译器无法对这两个模板进行“排序”，如果两个模板都适用，则会报告一个二义性错误。幸运的是，实现这一点并不难：我们可以在特化程度不好的模板上使用相同的 `EnableIf` 模式，并否定条件表达式。

### 提供多种特化
