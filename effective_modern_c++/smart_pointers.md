# 第四章 智能指针

## Item 18：对于独占资源使用 `std::unique_ptr`

- `std::unique_ptr` 是轻量级、快速的、只可移动（move-only）的管理专有所有权语义资源的智能指针
- 默认情况，资源销毁通过 `delete` 实现，但是支持自定义删除器。有状态的删除器和函数指针会增加 `std::unique_ptr` 对象的大小
- 将 `std::unique_ptr` 转化为 `std::shared_ptr` 非常简单

## Item 19：对于共享资源使用 `std::shared_ptr`

- `std::shared_ptr` 为有共享所有权的任意资源提供一种自动垃圾回收的便捷方式
- 较之于 `std::unique_ptr`，`std::shared_ptr` 对象通常大两倍，控制块会产生开销，需要原子性的引用计数修改操作
- 默认资源销毁是通过 `delete`，但是也支持自定义删除器。删除器的类型是什么对于 `std::shared_ptr` 的类型没有影响
- 避免从原始指针变量上创建 `std::shared_ptr`

## Item 20：当 `std::shared_ptr` 可能悬空时使用 `std::weak_ptr`

- 用 `std::weak_ptr` 替代可能会悬空的 `std::shared_ptr`
- `std::weak_ptr` 的潜在使用场景包括：缓存、观察者列表、打破 `std::shared_ptr` 环状结构

## Item 21：优先考虑使用 `std::make_unique` 和 `std::make_shared`，而非直接使用 `new`

- 和直接使用 `new` 相比，`make` 函数消除了代码重复，提高了异常安全性。对于 `std::make_shared` 和 `std::allocate_shared`，生成的代码更小更快
- 不适合使用 `make` 函数的情况包括需要指定自定义删除器和希望用花括号初始化
- 对于 `std::shared_ptr`，其他不建议使用 `make` 函数的情况包括
  1. 有自定义内存管理的类
  2. 特别关注内存的系统，非常大的对象，以及 `std::weak_ptr` 比对应的 `std::shared_ptr` 活得更久

## Item 22：当使用 Pimpl 惯用法，请在实现文件中定义特殊成员函数

- Pimpl 惯用法通过减少在类实现和类使用者之间的编译依赖来减少编译时间
- 对于 `std::unique_ptr` 类型的 `pImpl` 指针，需要在头文件的类里声明特殊的成员函数，但是在实现文件里面来实现他们。即使是编译器自动生成的代码可以工作，也要这么做。
- 以上的建议只适用于 `std::unique_ptr`，不适用于 `std::shared_ptr`
