@startuml

class Subject {
    Attach(Observer)
    Detach(Observer)
    Notify()
    __
}
class Observer {
    Update()
    __
}
class ConcreteSubject {
    GetState()
    SetState()
    __
    subjectState
}
class ConcreteObserver {
    Update()
    __
    observerState
}

Subject -> Observer : observer
Observer <|-- ConcreteObserver
ConcreteSubject <- ConcreteObserver : subject

note left of Subject::Notify
for all o in observers {
    o->Update()
}
end note

note left of ConcreteSubject
return subjectState
end note

note right of ConcreteObserver
observerState = subject->GetState()
end note

@enduml