@startuml

class Target {
    Request()
    __
}
class Adaptee {
    SpecificRequest()
    __
}
class Adapter {
    Request()
    __
}

Client -> Target
Target <|-- Adapter
Adaptee <|-- Adapter : (implementation)

note right of Adapter::Request
SpecificRequest()
end note

@enduml