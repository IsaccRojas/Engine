#include "implementations.hpp"

class ShrinkParticleAllocator : public ProvidedEntityAllocator<ShrinkParticle> { ShrinkParticle *_allocateProvided() override { return new ShrinkParticle; } };

class BulletAllocator : public ProvidedEntityAllocator<Bullet> {
    Provider<ShrinkParticle> *_shrinkparticle_provider;
    Bullet *_allocateProvided() override {
        Bullet *b = new Bullet;
        _shrinkparticle_provider->subscribe(b);
        return b;
    }
public:
    BulletAllocator(Provider<ShrinkParticle> *shrinkparticle_provider) : _shrinkparticle_provider(shrinkparticle_provider) {}
};

class PlayerAllocator : public ProvidedEntityAllocator<Player> {
    GLFWInput *_input;
    Provider<Bullet> *_bullet_provider;
    Player *_allocateProvided() override {
        Player *p = new Player(_input);
        _bullet_provider->subscribe(p);
        return p;
    }
public:
    PlayerAllocator(GLFWInput *input, Provider<Bullet> *bullet_provider) : _input(input), _bullet_provider(bullet_provider) {}
};

class EnemyAllocator : public EntityAllocatorInterface {
    Provider<Player> *_player_provider;
    bool *_killflag;
    Enemy *_allocate(int tag) override { 
        Enemy *e = new Enemy(_killflag); 
        _player_provider->subscribe(e);
        return e;
    }
public:
    EnemyAllocator(Provider<Player> *player_provider, bool *killflag) : _player_provider(player_provider), _killflag(killflag) {}
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