@startuml

class Product
class ConcreteProduct
class Creator {
    FactoryMethod()
    AnOperation()
    __
}
class ConcreteCreator {
    FactoryMethod()
    __
}

Product <|-- ConcreteProduct
Creator <|-- ConcreteCreator

ConcreteProduct <. ConcreteCreator

note right of Creator::AnOperation
...
product = FactoryMethod();
...
end note

note right of ConcreteCreator::FactoryMethod
return new ConcreteProduct
end note

@enduml