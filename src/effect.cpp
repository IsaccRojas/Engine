#include "effect.hpp"

void Effect::_initEntity() {
    _initEffect();

    getQuad()->bv_scale.v = _scale;
    _i = 0;
}

void Effect::_baseEntity() {
    _baseEffect();

    stepAnim();

    if (_lifetime >= 0) {
        _i++;
        if (_i >= _lifetime)
            kill();
    }

    // only queue if not set to be killed
    if (!getKillQueued())
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