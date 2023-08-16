@startuml

class Abstraction {
    Operation()
    __
}
class Implementor {
    OperationImp()
    __
}
class ConcreteImplementorA {
    OperationImp()
    __
}
class ConcreteImplementorB {
    OperationImp()
    __
}

Client --> Abstraction
Abstraction <|-- RefinedAbstraction
Abstraction o- Implementor
Implementor <|-- ConcreteImplementorA
Implementor <|-- ConcreteImplementorB

note left of Abstraction::Operation
imp->OperationImp();
end note

@enduml