#ifndef CHARACTER_HPP_
#define CHARACTER_HPP_

#include "basic.hpp"

class Character : public Basic {
    void _initBasic();
    void _baseBasic();
    void _killBasic();
    void _collisionBasic(Box *box);
    virtual void _initCharacter();
    virtual void _baseCharacter();
    virtual void _killCharacter();
    virtual void _collisionCharacter(Box *box);
public:
    Character(glm::vec3 scale);
};

#endif