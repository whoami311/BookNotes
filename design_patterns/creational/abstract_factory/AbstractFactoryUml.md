@startuml

abstract class AbstractFactory {
    CreateProductA()
    CreateProductB()
    __
}

class ConcreteFactory2 {
    CreateProductA()
    CreateProductB()
    __
}

class ConcreteFactory1 {
    CreateProductA()
    CreateProductB()
    __
}

class Client {}

abstract class AbstractProductA
class ProductA1
class ProductA2
abstract class AbstractProductB
class ProductB1
class ProductB2

AbstractFactory <|-- ConcreteFactory1
AbstractFactory <|-- ConcreteFactory2
AbstractFactory <- Client
Client --> AbstractProductA
AbstractProductA <|-- ProductA1
AbstractProductA <|-- ProductA2
Client --> AbstractProductB
AbstractProductB <|-- ProductB1
AbstractProductB <|-- ProductB2
ConcreteFactory1 ..> ProductA1
ConcreteFactory1 ..> ProductB1
ConcreteFactory2 ..> ProductA2
ConcreteFactory2 ..> ProductB2

@enduml