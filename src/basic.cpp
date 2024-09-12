#include "basic.hpp"

void Basic::_initObject() {
    // use scale to set box and quad
    getBox()->dim = _dimensions;
    getQuad()->bv_scale.v = _scale;

    _initBasic();
}

void Basic::_baseObject() {
    _baseBasic();

    // update quad to match box position, and step animation
    getQuad()->bv_pos.v = getBox()->pos;
    stepAnim();

    // only queue if not set to be killed
    if (!getKillEnqueued())
        enqueueExec(getLastExecQueue());
}

void Basic::_killObject() {
    getQuad()->bv_scale.v = glm::vec3(0.0f);
    _killBasic();
}

void Basic::_collisionObject(Box *box) {
    _collisionBasic(box);
}

Basic::Basic(glm::vec3 scale, glm::vec3 dimensions) : Object(), _scale(scale), _dimensions(dimensions) {}