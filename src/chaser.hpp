#ifndef ENEMY_HPP_
#define ENEMY_HPP_

#include "basic.hpp"

class Chaser : public Basic {
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

#endif