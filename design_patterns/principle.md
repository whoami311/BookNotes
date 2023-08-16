# 设计模式七大原则

## SOLID原则

solid 稳定的

### 单一职责原则（Single Responsibility Principle, SRP）

- 一个类应该仅有一个引起它变化的原因。
- 变化的方向隐含类的责任。

### 开放封闭原则（Open Closed Principle，OCP）

- 对扩展开放，对修改封闭。
- 类模块可扩展，但不可修改。

### 里氏替换原则（Liskov Substitution Principle，LSP）

- 子类必须能够替换它们的基类。
- 继承表达类型抽象。

### 接口隔离原则（Interface Segregation Principle，ISP）

- 不应该强迫客户程序依赖他们不用的方法。
- 接口应该小而完备。
- 一个接口应该只提供一种对外功能。

### 依赖倒置原则（Dependency Inversion Principle，DIP）

- 高层模块（稳定）不依赖低层模块（变化），两者依赖抽象（稳定）。
- 抽象（稳定）不依赖实现细节（变化），实现细节依赖抽象（稳定）。

## Other

### 最少知道原则（Least Knowledge Principle，LKP）

- 对象应该对其他对象尽可能少的了解。
- 各个模块之间互相调用时，通常会提供一个统一的接口来实现。

### 合成复用原则（Composite Reuse Principle，CARP）

- 优先使用对象组合而不是类继承。
- 类的继承通常是“白箱复用”，对象组合通常是“黑箱复用”。
- 继承在一定程序破坏封装性，子类和父类耦合度高。