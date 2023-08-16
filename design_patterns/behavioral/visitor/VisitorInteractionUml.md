@startuml

participant anObjectStructure
participant aConcreteElementA
participant aConcreteElementB
participant aConcreteVisitor

anObjectStructure -> aConcreteElementA : Accept(aVisitor)
aConcreteElementA -> aConcreteVisitor : VisitConcreteElementA(aConcreteElementA)
aConcreteVisitor -> aConcreteElementA : OperationA()
anObjectStructure -> aConcreteElementB : Accept(aVisitor)
aConcreteElementB -> aConcreteVisitor : VisitConcreteElementB(aConcreteElementB)
aConcreteVisitor -> aConcreteElementB : OperationB()

@enduml