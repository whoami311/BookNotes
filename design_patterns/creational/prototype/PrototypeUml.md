@startuml

class Client {
    Operation()
    __
}

class Prototype {
    Clone()
    __
}

class ConcretePrototype1 {
    Clone()
    __
}

class ConcretePrototype2 {
    Clone()
    __
}

Client -> Prototype
Prototype <|-- ConcretePrototype1
Prototype <|-- ConcretePrototype2

note left of Client::Operation
p = prototype->Clone()
end note

note right of ConcretePrototype1::Clone
return copy of self
end note

note right of ConcretePrototype2::Clone
return copy of self
end note

@enduml
