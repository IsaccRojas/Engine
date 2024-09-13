#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "effect.hpp"
#include "basic.hpp"
#include "../../../core/include/glfwinput.hpp"

enum Group { 
    T_BASIC_ORBSHOT, T_BASIC_PLAYER, T_BASIC_SMALLBALL, T_BASIC_MEDIUMBALL, T_BASIC_BIGBALL, T_BASIC_VERYBIGBALL, 
    T_EFFECT_SMALLSMOKE, T_EFFECT_MEDIUMSMOKE, T_EFFECT_BIGSMOKE, T_EFFECT_VERYBIGSMOKE, T_EFFECT_PLAYERSMOKE, T_EFFECT_ORBSHOTPARTICLE, T_EFFECT_ORBSHOTBOOM, T_EFFECT_BALLPARTICLE, T_EFFECT_RING
};

class SmallSmoke : public Effect {
public:
    SmallSmoke() : Effect(glm::vec3(16.0f, 16.0f, 1.0f), 20) {}
};

class MediumSmoke : public Effect {
public:
    MediumSmoke() : Effect(glm::vec3(24.0f, 24.0f, 1.0f), 20) {}
};

class BigSmoke : public Effect {
public:
    BigSmoke() : Effect(glm::vec3(26.0f, 26.0f, 1.0f), 20) {}
};

class VeryBigSmoke : public Effect {
public:
    VeryBigSmoke() : Effect(glm::vec3(28.0f, 28.0f, 1.0f), 20) {}
};

class PlayerSmoke : public Effect {
public:
    PlayerSmoke() : Effect(glm::vec3(16.0f, 16.0f, 1.0f), 20) {}
};

class OrbShotParticle : public Effect {
public:
    OrbShotParticle() : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 24) {}
};

class OrbShotBoom : public Effect {
public:
    OrbShotBoom() : Effect(glm::vec3(8.0f, 8.0f, 1.0f), 24) {}
};

class OrbShot : public Basic, public ProvidedType<OrbShot> {
    int _i;
    int _lifetime;
    glm::vec3 _direction;

    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);
public:
    OrbShot();
    void setDirection(glm::vec3 direction);
};

class Player : public Basic, public ProvidedType<Player>, public Receiver<OrbShot> {
    GLFWInput *_input;

    float _accel;
    float _deccel;
    float _spd_max;
    float _cooldown;
    float _max_cooldown;
    glm::vec2 _prevmovedir;
    glm::vec3 _dirvec;

    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);
    void _receive(OrbShot *orbshot) override;
public:
    Player(GLFWInput *input);

    void playerMotion();
    void playerAction();
};

class Chaser : public Basic, public Receiver<Player> {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;

    float _health;
    std::string _killeffect;
    bool *_killflag;

    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);

    Object *_getTarget();
public:
    Chaser(glm::vec3 scale, glm::vec3 dimensions, float health, std::string killeffect, bool *killflag);

    void chaserMotion();
    void chaserCollision();
};

class Ring : public Effect, public Receiver<Player> {
    void _initEffect() {
        // display beneath other objects
        getQuad()->bv_pos.v.z = -1.0f;
    }
    void _baseEffect() {
        int cyclestate = 0;

        // get first player found
        Player *p = nullptr;
        auto players = getAllProvided();
        if (players) {
            for (auto &player : *players) {
                p = player;
                break;
            }
        }

        // check player's distance from this instance's center
        if (p)
            if (glm::length(p->getBox()->pos - getQuad()->bv_pos.v) < 32.0f)
                cyclestate = 1;
                
        getAnimState().setCycleState(cyclestate);
    }
    void _killEffect() {}
public:
    Ring() : Effect(glm::vec3(64.0f, 64.0f, 1.0f), -1) {}
};

#endif