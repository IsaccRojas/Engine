#include "implementations.hpp"

class SmallBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(10.0f, 10.0f, 1.0f), 1, "SmallSmoke", _killflag); }
public:
    SmallBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class MediumBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(14.0f, 14.0f, 1.0f), 2, "MediumSmoke", _killflag); }
public:
    MediumBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class BigBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), glm::vec3(16.0f, 16.0f, 1.0f), 3, "BigSmoke", _killflag); }
public:
    BigBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class VeryBigBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), glm::vec3(20.0f, 20.0f, 1.0f), 4, "VeryBigSmoke", _killflag); }
public:
    VeryBigBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class OrbShotAllocator : public ObjectProvider<OrbShot> {};

class PlayerAllocator : public ObjectAllocatorInterface {
    GLFWInput *_input;
    OrbShotAllocator *_orbshot_allocator;
    Player *_allocate(int tag) override {
        Player *p = new Player(_input); 
        _orbshot_allocator->subscribe(p);
        return p;
    }
public:
    PlayerAllocator(GLFWInput *input, OrbShotAllocator *orbshot_allocator) : _input(input), _orbshot_allocator(orbshot_allocator) {}
};