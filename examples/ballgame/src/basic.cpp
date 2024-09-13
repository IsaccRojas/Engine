#include "basic.hpp"

void Basic::_initObject() {
    // use scale to set box and quad
    getBox()->dim = _dimensions;
    getQuad()->bv_scale.v = _scale;

    _initBasic();
}

void Basic::_baseObject() {
    _baseBasic();

    // update quad to match box position (except for z-coordinate), and step animation
    glm::vec2 boxpos = getBox()->pos;
    getQuad()->bv_pos.v = glm::vec3(boxpos, getQuad()->bv_pos.v.z);

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

void Basic::_initBasic() {}
void Basic::_baseBasic() {}
void Basic::_killBasic() {}
void Basic::_collisionBasic(Box *box) {}

Basic::Basic(glm::vec3 scale, glm::vec3 dimensions) : Object(), _scale(scale), _dimensions(dimensions) {}