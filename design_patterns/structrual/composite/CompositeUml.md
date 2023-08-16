@startuml

class Component {
    Operation()
    Add(Component)
    Remove(Component)
    GetChild(int)
    __
}
class Leaf {
    Operation()
    __
}
class Composite {
    Operation()
    Add(Component)
    Remove(Component)
    GetChild(int)
    __
}

Client -> Component
Component <|-- Leaf
Component <|-- Composite
Component --o Composite

note right of Composite::Operation
forall g in children
    g.OperationImp();
end note

@enduml