@startuml

class Aggregate {
    CreateIterator()
    __
}
class Client
class Iterator {
    First()
    Next()
    IsDone()
    CurrentItem()
    __
}
class ConcreteAggregate {
    CreateIterator()
    __
}
class ConcreteIterator

Aggregate <- Client
Client -> Iterator
Aggregate <|-- ConcreteAggregate
Iterator <|-- ConcreteIterator
ConcreteAggregate .> ConcreteIterator
ConcreteAggregate <- ConcreteIterator

note left of ConcreteAggregate::CreateIterator
return new ConcreteIterator(this)
end note

@enduml