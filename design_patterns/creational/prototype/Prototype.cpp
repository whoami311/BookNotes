// book
class MazeFactory {
public:
    MazeFactory();

    virtual Maze* MakeMaze() const {
        return new Maze;
    }
    virtual Wall* MakeWall() const {
        return new Wall;
    }
    virtual Room* MakeRoom(int n) const {
        return new Room(n);
    }
    virtual Door* MakeDoor(Room* r1, Room* r2) const {
        return new Door(r1, r2);
    }
};

class MazePrototypeFactory : public MazeFactory {
public:
    MazePrototypeFactory(Mazz*, Wall*, Room*, Door*);

    virtual Maze* MakeMaze() const;
    virtual Room* MakeRoom(int) const;
    virtual Wall* MakeWall() const;
    virtual Door* MakeDoor(Room*, Room*) const;

private:
    Maze* _prototypeMaze;
    Room* _prototypeRoom;
    Wall* _prototypeWall;
    Door* _prototypeDoor;
}

MazePrototypeFactory::MazePrototypeFactory(Maze* m, Wall w, Room* r, Door* d) {
    _prototypeMaze = m;
    _prototypeWall = w;
    _prototypeRoom = r;
    _prototypeDoor = d;
}

Wall* MazePrototypeFactory::MakeWall() const {
    return _prototypeWall->Clone();
}

Door* MazePrototypeFactory::MakeDoor(Room* r1, Room* r2) const {
    Door* door = _prototypeDoor->Clone();
    door->Initialize(r1, r2);
    return door;
}

MazeGame game;
MazePrototypeFactory simpleMazeFactory(new Maze, new Wall, new Room, new Door);
Maze* maze = game.CreateMaze(simpleMazeFactory);

MazePrototypeFactory bombedMazeFactory(new Maze, new BombedWall, new RoomWithABomb, new Door);

class Door : public MapSite {
public:
    Door();
    Door(const Door&);

    virtual void Initialize(Room*, Room*);
    virtual Door* Clone() const;
    virtual void Enter();
    Room* OtherSideFrom(Room*);

private:
    Room* _room1;
    Room* _room2;
};

Door::Door(const Door& other) {
    _room1 = other._room1;
    _room2 = other._room2;
}

void Door::Initialize(Room* r1, Room* r2) {
    _room1 = r1;
    _room2 = r2;
}

Door* Door::Clone() const {
    return new Door(*this);
}

class BombedWall : public Wall {
public:
    BombedWall();
    BombedWall(const BombedWall&);

    virtual Wall* Clone() const;
    bool HasBomb();

private:
    bool _bomb;
};

BombedWall::BombedWall(const BombedWall& other) : Wall(other) {
    _bomb = other._bomb;
}

Wall* BombedWall::Clone() const {
    return new BombedWall(*this);
}
