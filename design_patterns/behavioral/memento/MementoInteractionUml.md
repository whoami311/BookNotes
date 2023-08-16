@startuml

participant aCaretaker
participant anOriginator
participant aMemento

aCaretaker -> anOriginator : CreateMemento()
anOriginator --> aMemento : new Memento
anOriginator -> aMemento : SetState()

== ==

aCaretaker -> anOriginator : SetMemento(aMemento)
anOriginator -> aMemento : GetState()

@enduml