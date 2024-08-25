#ifndef SLIME_HPP_
#define SLIME_HPP_

#include "character.hpp"

class Slime : public Character {
    int _cur_animstate;
    int _prev_animstate;

    void _initCharacter();
    void _baseCharacter();
    void _killCharacter();
    void _collisionCharacter(Box *box);
public:
    Slime();

    void checkAnimState();
};

#endif