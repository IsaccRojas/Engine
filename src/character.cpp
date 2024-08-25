#include "character.hpp"

void Character::_initBasic() {
    _initCharacter();
}

void Character::_baseBasic() {
    _baseCharacter();
}

void Character::_killBasic() {
    _killCharacter();
}

void Character::_collisionBasic(Box *box) {
    _collisionCharacter(box);
}

void Character::_initCharacter() {}
void Character::_baseCharacter() {}
void Character::_killCharacter() {}
void Character::_collisionCharacter(Box *box) {}

Character::Character(glm::vec3 scale) : Basic(scale) {}