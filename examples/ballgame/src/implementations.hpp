#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "gfxball.hpp"
#include "physball.hpp"
#include "../../../core/include/glfwinput.hpp"

enum Group { 
    G_PHYSBALL_BULLET, G_PHYSBALL_PLAYER, G_PHYSBALL_ENEMY,
    G_GFXBALL_RING, G_GFXBALL_SHRINKPARTICLE
};

class ShrinkParticle;

class Bullet : public PhysBall, public ProvidedType<Bullet>, public Receiver<ShrinkParticle> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    void _initPhysBall();
    void _basePhysBall();
    void _killPhysBall();
    void _receive(ShrinkParticle *p) override;

public:
    Bullet();
    void setDirection(glm::vec3 direction);
};

class Player : public PhysBall, public ProvidedType<Player>, public Receiver<Bullet> {
    GLFWInput *_input;

    float _accel;
    float _deccel;
    float _spd_max;
    float _cooldown;
    float _max_cooldown;
    glm::vec2 _prevmovedir;
    glm::vec3 _dirvec;

    void _initPhysBall();
    void _basePhysBall();
    void _killPhysBall();
    void _receive(Bullet *bullet) override;

public:
    Player(GLFWInput *input);

    void playerMotion();
    void playerAction();
};

class Enemy : public PhysBall, public Receiver<Player> {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;

    float _health;
    bool *_killflag;

    void _initPhysBall();
    void _basePhysBall();
    void _killPhysBall();

    Entity *_getTarget();

public:
    Enemy(bool *killflag);

    void enemyMotion();
    void enemyCollision();
};

class Ring : public GfxBall, public Receiver<Player> {
    void _initGfxBall();
    void _baseGfxBall();
    void _killGfxBall();

public:
    Ring();
};

class ShrinkParticle : public GfxBall, public ProvidedType<ShrinkParticle> {
    void _initGfxBall();
    void _baseGfxBall();
    void _killGfxBall();

public:
    ShrinkParticle();
    glm::vec3 basescale;
    glm::vec4 color;
    unsigned lifetime;
};

#endif