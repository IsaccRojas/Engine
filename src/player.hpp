#ifndef PLAYER_HPP_
#define PLAYER_HPP_

#include "character.hpp"
#include "../core/include/input.hpp"

class Player : public Character {
    Input *_input;
    ObjectManager *_pm;

    bool _input_ready;
    float _accel;
    float _deccel;
    float _spd_max;
    float _cooldown;
    float _max_cooldown;

    void _initCharacter();
    void _baseCharacter();
    void _killCharacter();
    void _collisionCharacter(Box *box);

public:
    Player();

    void playerSetup(Input *input, ObjectManager *projectilemanager);
    void playerMotion();
    void playerAction();
};

#endif