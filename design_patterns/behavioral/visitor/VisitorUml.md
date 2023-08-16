@startuml

class Client

class Visitor {
    VisitConcreteElementA(ConcreteElementA)
    VisitConcreteElementB(ConcreteElementB)
    __
}

class ConcreteVisitorA {
    VisitConcreteElementA(ConcreteElementA)
    VisitConcreteElementB(ConcreteElementB)
    __
}

class ConcreteVisitorB {
    VisitConcreteElementA(ConcreteElementA)
    VisitConcreteElementB(ConcreteElementB)
    __
}

class ObjectStructure

class Element {
    Accept(Visitor)
    __
}

class ConcreteElementA {
    Accept(Visitor v)
    OperationA()
    __
}

class ConcreteElementB {
    Accept(Visitor v)
    OperationB()
    __
}

ConcreteElementA -[hidden]> ConcreteElementB

Client -> Visitor
Visitor <|-- ConcreteVisitorA
Visitor <|-- ConcreteVisitorB
Client --> ObjectStructure
ObjectStructure -> Element
Element <|-- ConcreteElementA
Element <|-- ConcreteElementB

note left of ConcreteElementA::Accept
v->VisitConcreteElementA(this)
end note

note right of ConcreteElementB::Accept
v->VisitConcreteElementB(this)
end note

@enduml