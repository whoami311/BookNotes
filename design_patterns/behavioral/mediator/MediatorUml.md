@startuml

class Mediator
class Colleague
class ConcreteMediator
class ConcreteColleague1
class ConcreteColleague2

Mediator <- Colleague : mediator
Mediator <|-- ConcreteMediator
Colleague <|-- ConcreteColleague1
Colleague <|-- ConcreteColleague2
ConcreteMediator -> ConcreteColleague1
ConcreteMediator -> ConcreteColleague2

@enduml