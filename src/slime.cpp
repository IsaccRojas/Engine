#include "slime.hpp"

void Slime::_initCharacter() {
    getBox()->setCorrection(true);
    getBox()->dim = glm::vec3(6.0f, 13.0f, 0.0f);
    getAnimState().setAnimState(0);
}

void Slime::_baseCharacter() {
    checkAnimState();
}

void Slime::_killCharacter() {}

void Slime::_collisionCharacter(Box *box) {}

Slime::Slime() : Character(glm::vec3(16.0f, 16.0f, 0.0f)), _cur_animstate(0), _prev_animstate(0) {}

void Slime::checkAnimState() {
    // do nothing if no change
    if (_cur_animstate == _prev_animstate)
        return;

    _prev_animstate = _cur_animstate;
}