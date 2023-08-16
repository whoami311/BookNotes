@startuml

class Director {
    Construct()
    __
}
class Builder {
    BuildPart()
    __
}
class ConcreteBuilder {
    BuildPart()
    GetResult()
    __
}
class Product

Director "builder" o- Builder
Builder <|-- ConcreteBuilder
ConcreteBuilder .> Product

note right of Director::Construct
for all objects in structure {
    builder->BuildPart()
}
end note

@enduml