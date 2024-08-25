#include "effect.hpp"

void Effect::_initEntity() {
    _initEffect();

    getQuad()->scale.v = _scale;
    getQuad()->pos.v.z = 1.0f;

    _i = 0;
}

void Effect::_baseEntity() {
    _baseEffect();

    stepAnim();

    _i++;
    if (_i >= _lifetime)
        kill();
    enqueue();
}

void Effect::_killEntity() {
    _killEffect();

    removeQuad();
}

void Effect::_initEffect() {}
void Effect::_baseEffect() {}
void Effect::_killEffect() {}

Effect::Effect(glm::vec3 scale, int lifetime) : Entity(), _scale(scale), _lifetime(lifetime), _i(0) {}