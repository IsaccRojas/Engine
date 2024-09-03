#include "basic.hpp"

void Basic::_initObject() {
    getBox()->dim = _scale;

    getQuad()->bv_scale.v = glm::vec3(_scale.x, _scale.y, 1.0f);
    getQuad()->bv_pos.v = getBox()->pos;

    _initBasic();
}

void Basic::_baseObject() {
    _baseBasic();

    getQuad()->bv_pos.v = getBox()->pos;

    stepAnim();
    enqueue();
}

void Basic::_killObject() {
    _killBasic();

    removeBox();
    removeQuad();
}

void Basic::_collisionObject(Box *box) {
    _collisionBasic(box);
    
    getQuad()->bv_pos.v = getBox()->pos;
}

void Basic::_initBasic() {}
void Basic::_baseBasic() {}
void Basic::_killBasic() {}
void Basic::_collisionBasic(Box *box) {}

Basic::Basic(glm::vec3 scale) : Object(), _scale(scale) {}