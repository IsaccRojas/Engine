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

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(ShrinkParticle *p) override;

public:
    Bullet();
    void setDirection(glm::vec3 direction);
};

class Player : public PhysBall, public ProvidedType<Player>, public Receiver<Bullet>, public Receiver<ShrinkParticle> {
    GLFWInput *_input;

    float _accel;
    float _deccel;
    float _spd_max;
    float _cooldown;
    float _max_cooldown;
    glm::vec2 _prevmovedir;
    glm::vec3 _dirvec;

    std::queue<glm::vec3> _deathparticledirs;

    void _initPhysBall() override;
    void _basePhysBall() override;
    void _killPhysBall() override;
    void _receive(Bullet *bullet) override;
    void _receive(ShrinkParticle *particle) override;

public:
    Player(GLFWInput *input);

    void playerMotion();
    void playerAction();
    void playerDeath();
};

class Enemy : public PhysBall, public ProvidedType<Enemy>, public Receiver<Player>, public Receiver<ShrinkParticle> {
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
    void _receive(Player *p) override;
    void _receive(ShrinkParticle *p) override;

    Entity *_getTarget();

public:
    Enemy(bool *killflag);

    void enemyMotion();
    void enemyCollision();
    void enemyDeath();
    void setHealth(float health);
};

class Ring : public GfxBall, public Receiver<Player> {
    void _initGfxBall() override;
    void _baseGfxBall() override;
    void _killGfxBall() override;

public:
    Ring();
};

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