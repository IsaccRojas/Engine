#include "implementations.hpp"

class OrbShotAllocator : public ProvidedObjectAllocator<OrbShot> { OrbShot *_allocateProvided() override { return new OrbShot; } };

class PlayerAllocator : public ProvidedObjectAllocator<Player> {
    GLFWInput *_input;
    Provider<OrbShot> *_orbshot_provider;
    Player *_allocateProvided() override {
        Player *p = new Player(_input);
        _orbshot_provider->subscribe(p);
        return p;
    }
public:
    PlayerAllocator(GLFWInput *input, Provider<OrbShot> *orbshot_provider) : _input(input), _orbshot_provider(orbshot_provider) {}
};

class SmallBallAllocator : public ObjectAllocatorInterface {
    Provider<Player> *_player_provider;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(10.0f, 10.0f, 1.0f), 1, "SmallSmoke", _killflag); 
        _player_provider->subscribe(c);
        return c;
    }
public:
    SmallBallAllocator(Provider<Player> *player_provider, bool *killflag) : _player_provider(player_provider), _killflag(killflag) {}
};

class MediumBallAllocator : public ObjectAllocatorInterface {
    Provider<Player> *_player_provider;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(14.0f, 14.0f, 1.0f), 2, "MediumSmoke", _killflag);
        _player_provider->subscribe(c);
        return c;
    }
public:
    MediumBallAllocator(Provider<Player> *player_provider, bool *killflag) : _player_provider(player_provider), _killflag(killflag) {}
};

class BigBallAllocator : public ObjectAllocatorInterface {
    Provider<Player> *_player_provider;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), glm::vec3(16.0f, 16.0f, 1.0f), 3, "BigSmoke", _killflag);
        _player_provider->subscribe(c);
        return c;
    }
public:
    BigBallAllocator(Provider<Player> *player_provider, bool *killflag) : _player_provider(player_provider), _killflag(killflag) {}
};

class VeryBigBallAllocator : public ObjectAllocatorInterface {
    Provider<Player> *_player_provider;
    bool *_killflag;
    Chaser *_allocate(int tag) override { 
        Chaser *c = new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), glm::vec3(20.0f, 20.0f, 1.0f), 4, "VeryBigSmoke", _killflag);
        _player_provider->subscribe(c);
        return c;
    }
public:
    VeryBigBallAllocator(Provider<Player> *player_provider, bool *killflag) : _player_provider(player_provider), _killflag(killflag) {}
};

class RingAllocator : public EntityAllocatorInterface {
    Provider<Player> *_player_provider;
    Ring *_allocate(int tag) override {
        Ring *r = new Ring;
        _player_provider->subscribe(r);
        return r;
    }
public:
    RingAllocator(Provider<Player> *player_provider) : _player_provider(player_provider) {}
};