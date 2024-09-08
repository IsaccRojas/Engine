#include "basic.hpp"

void Basic::_initObject() {
    // use scale to set box and quad
    getBox()->dim = _scale;
    getQuad()->bv_scale.v = glm::vec3(_scale.x, _scale.y, 1.0f);

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
    _killBasic();
}

void Basic::_collisionObject(Box *box) {
    _collisionBasic(box);
}

void Basic::_initBasic() {}
void Basic::_baseBasic() {}
void Basic::_killBasic() {}
void Basic::_collisionBasic(Box *box) {}

Basic::Basic(glm::vec3 scale) : Object(), _scale(scale) {}