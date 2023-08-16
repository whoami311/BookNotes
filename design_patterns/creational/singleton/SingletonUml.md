@startuml
class Singleton {
    {static} static Instance()
    SingletonOperation()
    GetSingletonData()
    __
    {static} static uniqueInstance
    singletonData
}

note right of Singleton::Instance()
    return uniqueInstance
end note
@enduml