@startuml

class Originator {
    SetMemento(Memento m)
    CreateMemento()
    __
    state
}
class Memento {
    GetState()
    SetState()
    __
    state
}
class Caretaker

Originator .> Memento
Memento -o Caretaker : memento

note left of Originator::SetMemento
state = m->GetState()
end note

note left of Originator::CreateMemento
return new Memento(state)
end note

@enduml