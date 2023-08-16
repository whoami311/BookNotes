@startuml

class Client
class Handler {
    HandleRequest()
    __
}
class ConcreteHandler1 {
    HandleRequest()
    __
}
class ConcreteHandler2 {
    HandleRequest()
    __
}

Client -> Handler
Handler -> Handler : successor
Handler <|-- ConcreteHandler1
Handler <|-- ConcreteHandler2

@enduml