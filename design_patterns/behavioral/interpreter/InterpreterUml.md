@startuml

class Context
class Client
class AbstractExpression {
    Interpret(Context)
    __
}
class TerminalExpression {
    Interpret(Context)
    __
}
class NonterminalExpression {
    Interpret(Context)
    __
}

Context <-- Client
Client -> AbstractExpression
AbstractExpression <|-- TerminalExpression
AbstractExpression <|-- NonterminalExpression
NonterminalExpression o-- AbstractExpression

@enduml