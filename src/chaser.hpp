#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "character.hpp"

class Chaser : public Character {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;
    Object *_target;

    float _health;
    std::string _killeffect;

    void _initCharacter();
    void _baseCharacter();
    void _killCharacter();
    void _collisionCharacter(Box *box);

public:
    Chaser(float health, std::string killeffect);

    void chaserMotion();
    void chaserSetTarget(Object *target);
};

#endif