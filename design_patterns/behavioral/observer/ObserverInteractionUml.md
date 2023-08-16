@startuml

aConcreteSubject <- aConcreteObserver : SetState()
aConcreteSubject -> aConcreteSubject : Notify()
aConcreteSubject -> aConcreteObserver : Update()

skinparam sequenceMessageAlign center

aConcreteObserver -> aConcreteSubject : GetState()
aConcreteSubject -> anotherConcreteObserver : Update()
anotherConcreteObserver -> aConcreteSubject : GetState()

@enduml