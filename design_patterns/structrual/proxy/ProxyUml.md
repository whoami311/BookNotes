@startuml

class Client
class Subject {
    Request()
    ...
    __
}
class RealSubject {
    Request()
    ...
    __
}
class Proxy {
    Request()
    ...
    __
}

Client -> Subject
Subject <|-- RealSubject
Subject <|-- Proxy
RealSubject <- Proxy : realSubject

note right of Proxy::Request
...
realSubject->Request();
...
end note

@enduml