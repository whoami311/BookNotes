@startuml

class Client
class Invoker
class Command {
    Execute()
    __
}
class Receiver {
    Action()
    __
}
class ConcreteCommand {
    Execute()
    __
    state
}

Client -> Receiver
Invoker o- Command
Command <|-- ConcreteCommand
Receiver <- ConcreteCommand : receiver
Client .> ConcreteCommand

note right of ConcreteCommand::Execute
receiver->Action;
end note

@enduml