#include "entity.hpp"

Entity::Entity() : _ready(false), _firststep(true), _quadid(-1), _glenv(nullptr), _quad(nullptr), _frame(nullptr) {}
Entity::~Entity() {
    if (_quadid >= 0) {
        // quad was not removed by a call to _kill(), must remove it here
        _glenv->erase(_quadid);
    }
}

void Entity::_setup(GLEnv *glenv, Animation *animation) {
    _glenv = glenv;
    _animstate.setAnimation(animation);
    
    // get quad data
    _quadid = _glenv->genQuad(glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), glm::vec2(0.0f));
    _quad = _glenv->get(_quadid);

    _frame = _animstate.getCurrent();

    _ready = true;
}

void Entity::_init() {
    if (_ready)
        _initEntity();
}

void Entity::_base() {
    if (_ready) {
        _baseEntity();

        if (!_firststep) {
            // step animation and retrieve current frame
            _animstate.step();
            _frame = _animstate.getCurrent();
        } else {
            _firststep = false;
        }

        // write frame data to quad and update quad
        _quad->pos.v = pos;
        _quad->texpos.v = _frame->texpos;
        _quad->texsize.v = _frame->texsize;
        _quad->update();
    }
}

void Entity::_kill() {
    if (_ready) {
        _killEntity();

        // erase quad
        _glenv->erase(_quadid);

        // set quad ID to invalid value
        _quadid = -1;
    }
}

void Entity::_initEntity() {}
void Entity::_baseEntity() {}
void Entity::_killEntity() {}

Quad *Entity::quad() { return _quad; }
