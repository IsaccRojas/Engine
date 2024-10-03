#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "gfxball.hpp"
#include "physball.hpp"
#include "../../../core/include/glfwinput.hpp"

enum Group { 
    G_PHYSBALL_BULLET, G_PHYSBALL_BOMB, G_PHYSBALL_EXPLOSION, G_PHYSBALL_PLAYER, G_PHYSBALL_ENEMY,
    G_GFXBALL_RING, G_GFXBALL_SHRINKPARTICLE
};

class Bullet;
class Bomb;
class Explosion; // does not hold Providers reference
class Player;
class Enemy;
class Ring;
class ShrinkParticle; // does not hold Providers reference

// struct to group providers together
struct Providers {
    Provider<Bullet> Bullet_provider;
    Provider<Bomb> Bomb_provider;
    Provider<Explosion> Explosion_provider;
    Provider<Player> Player_provider;
    Provider<Enemy> Enemy_provider;
    Provider<Ring> Ring_provider;
    Provider<ShrinkParticle> ShrinkParticle_provider;
};

// small interface to implement obtaining the providers struct
class ProvidersHolder {
    Providers *_providers;
public:
    ProvidersHolder(Providers *providers) : _providers(providers) {}
    Providers &providers() { return *_providers; }
};

// generic template for ProvidedEntityAllocators that need a reference to a Providers struct
template<class T>
class HolderAllocator : public ProvidedEntityAllocator<T> {
    Providers *_providers;
    T *_allocateProvided() override { return new T(_providers); }
public:
    HolderAllocator(Providers *providers) : _providers(providers) {}
};

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle
class Bullet : public PhysBall, public ProvidedType<Bullet>, public ProvidersHolder, public Receiver<ShrinkParticle> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;

public:
    Bullet(Providers *providers = nullptr);
    void setDirection(glm::vec3 direction);
};

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle and Explosion
class Bomb : public PhysBall, public ProvidedType<Bomb>, public ProvidersHolder, public Receiver<ShrinkParticle>, public Receiver<Explosion> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;
    void _receive(Explosion *e) override;

public:
    Bomb(Providers *providers = nullptr);
    void setDirection(glm::vec3 direction);
};

// implements PhysBall, and is a provided type
class Explosion : public PhysBall, public ProvidedType<Explosion> {
    float _base_innerrad;
    float _base_outerrad;
    glm::vec4 _color;
    glm::vec3 _vel;
    float _rate_inner;
    float _rate_outer;
    unsigned _lifetime;
    unsigned _i;
    unsigned _active_time;
    unsigned _update_rate;


    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;

public:
    Explosion();
    void set(float base_innerrad, float base_outerrad, glm::vec4 color, unsigned lifetime, glm::vec3 vel, float rate_inner, float rate_outer, unsigned update_rate);
};

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive Bullet, Bomb, and ShrinkParticle
class Player : public PhysBall, public ProvidedType<Player>, public ProvidersHolder, public Receiver<Bullet>, public Receiver<Bomb>, public Receiver<ShrinkParticle> {
    GLFWInput *_input;

    float _accel;
    float _deccel;
    float _spd_max;
    float _bullet_cooldown;
    float _bomb_cooldown;
    float _bullet_cooldown_max;
    float _bomb_cooldown_max;
    glm::vec2 _prevmovedir;
    glm::vec3 _dirvec;

    std::queue<glm::vec3> _deathparticledirs;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(Bullet *bullet) override;
    void _receive(Bomb *bomb) override;
    void _receive(ShrinkParticle *particle) override;

public:
    Player(Providers *providers = nullptr);

    void playerMotion();
    void playerAction();
    void playerDeath();

    void set(GLFWInput *input);
};

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle
class Enemy : public PhysBall, public ProvidedType<Enemy>, public ProvidersHolder, public Receiver<ShrinkParticle> {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;

    float _health;
    bool *_killflag;

    std::queue<glm::vec3> _deathparticledirs;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;

    Entity *_getTarget();

public:
    Enemy(Providers *providers = nullptr);

    void enemyMotion();
    void enemyCollision();
    void enemyDeath();

    void setHealth(float health);
    void setKillFlag(bool *killflag);
};

// implements GfxBall, is a provided type, and needs access to Providers
class Ring : public GfxBall, public ProvidedType<Ring>, public ProvidersHolder {
    void _initGfxBall() override;
    void _baseGfxBall() override;
    void _killGfxBall() override;

public:
    Ring(Providers *providers = nullptr);
};

// implements GfxBall, and is a provided type
class ShrinkParticle : public GfxBall, public ProvidedType<ShrinkParticle> {
    glm::vec3 _basescale;
    glm::vec4 _color;
    glm::vec3 _vel;
    
    void _initGfxBall() override;
    void _baseGfxBall() override;
    void _killGfxBall() override;

public:
    ShrinkParticle();
    void set(glm::vec3 basescale, glm::vec4 color, unsigned lifetime, glm::vec3 vel);
};

#endif