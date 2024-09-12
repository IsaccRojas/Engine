#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "basic.hpp"
#include "../core/include/input.hpp"

class OrbShot : public Basic {
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

class Player : public Basic, public Receiver<OrbShot> {
    Input *_input;

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
    Player(Input *input);

    void playerMotion();
    void playerAction();
};

#endif