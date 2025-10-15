#include "object.hpp"

void Object::_initEntity() {
    // create and set quad and box
    _quad_off = executor().glenv().genQuad(transform.pos, transform.scale, glm::vec4(1.0f), 0.0f, glm::vec3(0.0f), glm::vec2(0.0f), GLE_RECT);
    _quad = executor().glenv().getQuad(_quad_off);
    _box = executor().boxspace().push(transform, glm::vec3(0.0f), nullptr);
    _box->transform = transform;

    // set animation and filter if they are named
    if (_animation_name != "")
        _quad->animationstate().setAnimation(&executor().animations()[_animation_name]);
    if (_filter_name != "")
        _box->filterstate().setFilter(&executor().filters()[_filter_name]);
    
    // bind and set collision handler, and set default object weight
    _box->setCallback(std::bind(&Object::_onCollision, this, std::placeholders::_1));
    _box->attributes["weight"] = 0.0f;

    _initObject();
}

void Object::_baseEntity() {
    _baseObject();

    // update transform with velocity
    transform.pos += vel;

    // set box transform to be equal to Script
    _box->transform = transform;

    // update quad to match (except for z-coordinate)
    _quad->bv_pos.v = glm::vec3(transform.pos.x, transform.pos.y, _quad->bv_pos.v.z);
    _quad->bv_scale.v = transform.scale;

    if (_quad->animationstate().hasAnimation()) {
        _quad->animationstate().step();
        _quad->writeAnimation();
    }

    // only queue if not set to be killed
    if (!getKillEnqueued())
        enqueueExec(getLastExecQueue());
}

void Object::_killEntity() {
    executor().glenv().remove(_quad_off);
    executor().boxspace().erase(_box);

    _killObject();
}

void Object::_initObject() {}
void Object::_baseObject() {}
void Object::_killObject() {}
void Object::_onCollision(Box *other) {}

Object::Object(std::string animation_name, std::string filter_name) : 
    Entity(), 
    _quad(nullptr),
    _box(nullptr),
    _animation_name(animation_name),
    _filter_name(filter_name),
    vel(glm::vec3(0.0f))
{}

Quad *Object::quad() { return _quad; }
Box *Object::box() { return _box; }