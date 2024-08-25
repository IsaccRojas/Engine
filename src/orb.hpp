#ifndef ORB_HPP_
#define ORB_HPP_

#include "basic.hpp"
#include "../core/include/input.hpp"

class Orb : public Basic {
    Input *_input;
    Basic *_source;
    ObjectManager *_pm;
    bool _orb_ready;

    float _radius;
    float _cooldown;
    float _max_cooldown;

    float _anim_cooldown;
    float _anim_max_cooldown;

    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);
public:
    Orb();

    void orbSetup(Input *input, Basic *source, ObjectManager *projectilemanager);
};

#endif