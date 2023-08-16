@startuml

class Context {
    Request()
    __
}

class State {
    Handle()
    __
}

class ConcreteStateA {
    Handle()
    __
}

class ConcreteStateB {
    Handle()
    __
}

Context "state" o-- State
State <|-- ConcreteStateA
State <|-- ConcreteStateB

note right of Context::Request()
    state->Handle()
end note

@enduml