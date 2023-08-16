@startuml

class Component {
    Operation()
    __
}
class ConcreteComponent {
    Operation()
    __
}
class Decorator {
    Operation()
    __
}
class ConcreteDecoratorA {
    Operation()
    __
    addedState
}
class ConcreteDecoratorB {
    Operation()
    AddedBehavior()
    __
}

Component <|-- ConcreteComponent
Component <|-- Decorator
Decorator <|-- ConcreteDecoratorA
Decorator <|-- ConcreteDecoratorB
Decorator o-- Component : "component"

note right of Decorator::Operation
conponent->Operation()
end note

note right of ConcreteDecoratorB::Operation
Decorator::Operation();
AddedBehavior();
end note

@enduml