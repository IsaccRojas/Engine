#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "gfxball.hpp"
#include "physball.hpp"
#include "../../../core/include/glfwinput.hpp"
#include "../../../core/include/text.hpp"

enum Group { 
    G_PHYSBALL_BULLET, G_PHYSBALL_BOMB, G_PHYSBALL_EXPLOSION, G_PHYSBALL_PLAYER, G_PHYSBALL_ENEMY,
    G_GFXBALL_RING, G_GFXBALL_SHRINKPARTICLE
};

// --------------------------------------------------------------------------------------------------------------------------

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

// holds global state, and receives enemies and players to initialize them
class GlobalState : public Receiver<Enemy> {
    // set health
    void _receive(Enemy *enemy) override;

public:
    Providers providers;

    int game_state;
    int i;

    float number;
    float rate;
    float target_number;
    float max_rate;
    float rate_increase;
    float rate_decrease;
    
    int round;
    int spawn_rate;

    bool enter_check;
    bool enter_state;

    GLFWInput *input;
    bool killflag;

    Text toptext;
    Text subtext;
    Text bottomtext;

    std::queue<int> size_factors;
    std::vector<int> upgrade_counts;
    std::vector<Text> upgrade_texts;

    // need this to initialize Text members
    GlobalState(GLEnv *glenv, GLFWInput *glfwinput);
    ~GlobalState();
    
    /* Resets all state. */
    void reset();
    
    /* Transitions from one state to another. */
    void transition(int state);
};

// small interface to implement obtaining the global state reference
class StateReferrer {
    GlobalState *_globalstate;
public:
    StateReferrer(GlobalState *globalstate) : _globalstate(globalstate) {}
    GlobalState &globalstate() { return *_globalstate; }
};

// generic template for ProvidedEntityAllocators that need a reference to a GlobalState instance
template<class T>
class ReferrerAllocator : public ProvidedEntityAllocator<T> {
    GlobalState *_globalstate;
    T *_allocateProvided() override { return new T(_globalstate); }
public:
    ReferrerAllocator(GlobalState *globalstate) : _globalstate(globalstate) {}
};

// --------------------------------------------------------------------------------------------------------------------------

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle
class Bullet : public PhysBall, public ProvidedType<Bullet>, public StateReferrer, public Receiver<ShrinkParticle> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    float _health;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;

public:
    Bullet(GlobalState *globalstate = nullptr);
    void set(glm::vec3 direction, float health);
};

// --------------------------------------------------------------------------------------------------------------------------

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle and Explosion
class Bomb : public PhysBall, public ProvidedType<Bomb>, public StateReferrer, public Receiver<ShrinkParticle>, public Receiver<Explosion> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;
    void _receive(Explosion *e) override;

public:
    Bomb(GlobalState *globalstate = nullptr);
    void setDirection(glm::vec3 direction);
};

// --------------------------------------------------------------------------------------------------------------------------

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

// --------------------------------------------------------------------------------------------------------------------------

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive Bullet, Bomb, and ShrinkParticle
class Player : public PhysBall, public ProvidedType<Player>, public StateReferrer, public Receiver<Bullet>, public Receiver<Bomb>, public Receiver<ShrinkParticle> {
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
    Player(GlobalState *globalstate = nullptr);

    void playerMotion();
    void playerAction();
    void playerDeath();
};

// --------------------------------------------------------------------------------------------------------------------------

// implements PhysBall, is a provided type, needs access to Providers, and needs to receive ShrinkParticle
class Enemy : public PhysBall, public ProvidedType<Enemy>, public StateReferrer, public Receiver<ShrinkParticle> {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;

    float _health;
    float _max_health;

    std::queue<glm::vec3> _deathparticledirs;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;

    Entity *_getTarget();

public:
    Enemy(GlobalState *globalstate = nullptr);

    void enemyMotion();
    void enemyCollision();
    void enemyDeath();

    void set(float health);
};

// --------------------------------------------------------------------------------------------------------------------------

// implements GfxBall, is a provided type, and needs access to Providers
class Ring : public GfxBall, public ProvidedType<Ring>, public StateReferrer {
    void _initGfxBall() override;
    void _baseGfxBall() override;
    void _killGfxBall() override;

public:
    Ring(GlobalState *globalstate = nullptr);
};

// --------------------------------------------------------------------------------------------------------------------------

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