#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "character.hpp"

class Chaser : public Character {
    float _accel;
    float _deccel;
    float _spd_max;
    float _t;
    glm::vec3 _prevdir;

    float _health;
    std::string _killeffect;
    bool *_killflag;

    void _initCharacter();
    void _baseCharacter();
    void _killCharacter();
    void _collisionCharacter(Box *box);

    Object *_getTarget();
public:
    Chaser(glm::vec3 scale, float health, std::string killeffect, bool *killflag);

    void chaserMotion();
};

#endif