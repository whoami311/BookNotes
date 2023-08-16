# Abstract Factory

## 意图

提供一个接口以创建一系列相关或相互依赖的对象，而无须指定它们具体的类。

## 动机

客户仅与抽象类定义的接口交互，而不使用特定的具体类的接口。

## 适用性

- 一个系统要独立于它的产品的创建、组合和表示。
- 一个系统要由多个产品系列中的一个来配置。
- 要强调一系列相关的产品对象的设计以便进行联合使用。
- 提供一个产品类库，但只想显示它们的接口而不是实现。

## 结构

![AbstractFactory](AbstractFactory.png)

## 参与者

- AbstractFactory（WidgetFactory）

—— 声明一个创建抽象产品对象的操作接口。

- ConcreteFactory（ModifyWidgetFactory、PMWidgetFactory）

—— 实现创建具体产品对象的操作。

- AbstractProduct（Window、ScrollBar）

—— 为一类产品对象声明一个接口。

- ConcreteProduct（MotifWindow、MotifScrollBar）

—— 定义一个将被相应的具体工厂创建的产品对象。
—— 实现AbstractProduct接口。

- Client

—— 仅使用由AbstractFactory和AbstractProduct类声明的接口。

## 协作

- 通常在运行时创建一个ConcreteFactory类的实例，这一具体的工厂创建具有特定实现的产品对象。为创建不同的产品对象，客户应使用不同的具体工厂。
- AbstractFactory将产品对象的创建延迟到它的ConcreteFactory子类。

## 效果

1. 它分离了具体的类。
2. 它使得易于交换产品系列。
3. 它有利于产品的一致性。
4. 难以支持新种类的产品。

## 实现

1. 将工厂作为单件。
2. 创建产品。
3. 定义可扩展的工厂。