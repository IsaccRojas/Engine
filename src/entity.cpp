#include "entity.hpp"

Entity::Entity() : _setup_ready(false), _first_step(true), _quad_id(-1), _glenv(nullptr), _quad(nullptr), _frame(nullptr), Script() {}
Entity::~Entity() {
    // try erasing existing quad
    this->eraseQuad();
}

void Entity::_init() {
    _initEntity();
}

void Entity::_base() {
    _baseEntity();

    if (_setup_ready) {

        if (!_first_step) {
            // step animation and retrieve current frame
            _animstate.step();
            _frame = _animstate.getCurrent();
        } else {
            _first_step = false;
        }

        // only attempt to write animation data if quad is available
        if (_quad_ready) {
            // write frame data to quad and update quad
            _quad->pos.v = _visualpos;
            _quad->texpos.v = _frame->texpos;
            _quad->texsize.v = _frame->texsize;
            _quad->update();
        }

    }
}

void Entity::_kill() {
    _killEntity();
}

void Entity::_initEntity() {}
void Entity::_baseEntity() {}
void Entity::_killEntity() {}

void Entity::entitySetup(GLEnv *glenv, Animation *animation) {
    // try erasing existing quad
    this->eraseQuad();

    _glenv = glenv;

    _animstate.setAnimation(animation);
    _frame = _animstate.getCurrent();

    _setup_ready = true;
}

void Entity::genQuad(glm::vec3 pos, glm::vec3 scale) {
    if (_setup_ready) {
        // erase existing quad
        if (_quad_ready)
            this->eraseQuad();

        // get quad data
        _quad_id = _glenv->genQuad(pos, scale, _frame->texpos, _frame->texsize);
        
        // if successful, retrieve quad
        if (_quad_id >= 0) {
            _quad = _glenv->get(_quad_id);
            _quad_ready = true;
        }
    }
}

void Entity::eraseQuad() {
    if (_quad_id >= 0) {
        _glenv->erase(_quad_id);
        _quad_id = -1;

        _quad_ready = false;
    }
}

Quad *Entity::getQuad() { 
    if (_quad_ready)
        return _quad;
    else
        return NULL;
}