@startuml

class FlyweightFactory {
    GetFlyweight(key)
    __
}
class Flyweight {
    Operation(extrinsicState)
    __
}
class Client
class ConcreteFlyweight {
    Operation(extrinsicState)
    __
    intrinsicState
}
class UnsharedConcreteFlyweight {
    Operation(extrinsicState)
    __
    allState
}

FlyweightFactory o- Flyweight : flyweights
FlyweightFactory <-- Client
Client -> ConcreteFlyweight
Client -> UnsharedConcreteFlyweight
Flyweight <|-- ConcreteFlyweight
Flyweight <|-- UnsharedConcreteFlyweight

note left of FlyweightFactory::GetFlyweight
if (flyweight[key] exists) {
    return existing flyweight;
} else {
    create new flyweight;
    add it to pool of flyweights;
    return the new flyweight;
}
end note

@enduml