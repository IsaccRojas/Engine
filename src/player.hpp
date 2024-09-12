#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "basic.hpp"
#include "../core/include/input.hpp"

class Player : public Basic {
    Input *_input;

    float _accel;
    float _deccel;
    float _spd_max;
    float _cooldown;
    float _max_cooldown;
    glm::vec2 _prevmovedir;

    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);

public:
    Player(Input *input);

    void playerMotion();
    void playerAction();
};

#endif