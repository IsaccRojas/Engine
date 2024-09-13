#include "implementations.hpp"

class OrbShotAllocator : public ObjectProvider<OrbShot> {
    virtual OrbShot *_allocateInstance() override { return new OrbShot; }
};

class PlayerAllocator : public ObjectProvider<Player> {
    GLFWInput *_input;
    OrbShotAllocator *_orbshot_allocator;
    Player *_allocateInstance() override {
        Player *p = new Player(_input); 
        _orbshot_allocator->subscribe(p);
        return p;
    }
public:
    PlayerAllocator(GLFWInput *input, OrbShotAllocator *orbshot_allocator) : _input(input), _orbshot_allocator(orbshot_allocator) {}
};

class SmallBallAllocator : public ObjectAllocatorInterface {
    PlayerAllocator *_player_allocator;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(10.0f, 10.0f, 1.0f), 1, "SmallSmoke", _killflag); 
        _player_allocator->subscribe(c);
        return c;
    }
public:
    SmallBallAllocator(PlayerAllocator *player_allocator, bool *killflag) : _player_allocator(player_allocator), _killflag(killflag) {}
};

class MediumBallAllocator : public ObjectAllocatorInterface {
    PlayerAllocator *_player_allocator;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(14.0f, 14.0f, 1.0f), 2, "MediumSmoke", _killflag);
        _player_allocator->subscribe(c);
        return c;
    }
public:
    MediumBallAllocator(PlayerAllocator *player_allocator, bool *killflag) : _player_allocator(player_allocator), _killflag(killflag) {}
};

class BigBallAllocator : public ObjectAllocatorInterface {
    PlayerAllocator *_player_allocator;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), glm::vec3(16.0f, 16.0f, 1.0f), 3, "BigSmoke", _killflag);
        _player_allocator->subscribe(c);
        return c;
    }
public:
    BigBallAllocator(PlayerAllocator *player_allocator, bool *killflag) : _player_allocator(player_allocator), _killflag(killflag) {}
};

class VeryBigBallAllocator : public ObjectAllocatorInterface {
    PlayerAllocator *_player_allocator;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), glm::vec3(20.0f, 20.0f, 1.0f), 4, "VeryBigSmoke", _killflag);
        _player_allocator->subscribe(c);
        return c;
    }
public:
    VeryBigBallAllocator(PlayerAllocator *player_allocator, bool *killflag) : _player_allocator(player_allocator), _killflag(killflag) {}
};

class RingAllocator : public EntityAllocatorInterface {
    PlayerAllocator *_player_allocator;
    Ring *_allocate(int tag) override {
        Ring *r = new Ring;
        _player_allocator->subscribe(r);
        return r;
    }
public:
    RingAllocator(PlayerAllocator *player_allocator) : _player_allocator(player_allocator) {}
};