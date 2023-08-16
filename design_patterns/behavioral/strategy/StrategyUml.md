@startuml

class Context {
    ContextInterface()
    __
}
class Strategy {
    AlgorithmInterface()
    __
}
class ConcreteStrategyA {
    AlgorithmInterface()
    __
}
class ConcreteStrategyB {
    AlgorithmInterface()
    __
}
class ConcreteStrategyC {
    AlgorithmInterface()
    __
}

Context <|- Strategy : strategy
Strategy <|-- ConcreteStrategyA
Strategy <|-- ConcreteStrategyB
Strategy <|-- ConcreteStrategyC

@enduml