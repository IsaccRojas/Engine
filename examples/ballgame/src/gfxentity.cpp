#include "gfxentity.hpp"

void GfxEntity::_initEntity() {
    // create and set quad
    _quad_off = executor().glenv().genQuad(transform.pos, transform.scale, glm::vec4(1.0f), 0.0f, glm::vec3(0.0f), glm::vec2(0.0f), _quad_type);
    _quad = executor().glenv().getQuad(_quad_off);

    // set animation if it is named
    if (_animation_name != "")
        _quad->animationstate().setAnimation(&executor().animations()[_animation_name]);

    _i = 0;
    _initGfxEntity();
}

void GfxEntity::_baseEntity() {
    _baseGfxEntity();

    // update quad to match Script transform (except for z-coordinate), and step animation
    _quad->bv_pos.v = glm::vec3(transform.pos.x, transform.pos.y, _quad->bv_pos.v.z);
    _quad->bv_scale.v = transform.scale;

    if (_quad->animationstate().hasAnimation()) {
        _quad->animationstate().step();
        _quad->writeAnimation();
    }

    if (_lifetime >= 0) {
        _i++;
        if (_i >= _lifetime)
            enqueueKill();
    }

    // only queue if not set to be killed
    if (!getKillEnqueued())
        enqueueExec(getLastExecQueue());
}

void GfxEntity::_killEntity() {
    executor().glenv().remove(_quad_off);

    _killGfxEntity();
}

void GfxEntity::_initGfxEntity() {}
void GfxEntity::_baseGfxEntity() {}
void GfxEntity::_killGfxEntity() {}

GfxEntity::GfxEntity(std::string animation_name, int lifetime, DrawType type) : 
    Entity(),
    _quad_type(type),
    _quad(nullptr),
    _quad_off(0),
    _animation_name(animation_name),
    _lifetime(lifetime),
    _i(0)
{}

Quad *GfxEntity::quad() { return _quad; }