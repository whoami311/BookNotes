@startuml

class AbstractClass {
    TemplateMethod()
    PrimitiveOperation1()
    PrimitiveOperation2()
    __
}

class ConcreteClass {
    PrimitiveOperation1()
    PrimitiveOperation2()
    __
}

AbstractClass <|-- ConcreteClass

note right of AbstractClass::TemplateMethod
...
PrimitiveOperation1()
...
PrimitiveOperation2()
...
end note

@enduml