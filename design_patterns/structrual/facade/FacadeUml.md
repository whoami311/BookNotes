@startuml

class Client
class Facade
class SubsystemClassA
class SubsystemClassB

Client --> Facade
Facade --> SubsystemClassA
Facade --> SubsystemClassB

@enduml