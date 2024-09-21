#include "gfxball.hpp"

void GfxBall::_initEntity() {
    // create and set quad
    _quad = executor().glenv().getQuad(
        executor().glenv().genQuad(transform().pos, transform().scale, glm::vec4(1.0f), glm::vec3(0.0f), glm::vec2(0.0f), GLE_ELLIPSE)
    );

    // set animation if it is named
    if (_animation_name != "")
        _quad->setAnim(&executor().animations()[_animation_name]);

    _i = 0;
    _initGfxBall();
}

void GfxBall::_baseEntity() {
    _baseGfxBall();

    // update quad to match Script transform (except for z-coordinate), and step animation
    _quad->bv_pos.v = glm::vec3(transform().pos.x, transform().pos.y, _quad->bv_pos.v.z);

    _quad->stepAnim();

    if (_lifetime >= 0) {
        _i++;
        if (_i >= _lifetime)
            enqueueKill();
    }

    // only queue if not set to be killed
    if (!getKillEnqueued())
        enqueueExec(getLastExecQueue());
}

void GfxBall::_killEntity() {
    _quad->bv_scale.v = glm::vec3(0.0f);
    _killGfxBall();
}

void GfxBall::_initGfxBall() {}
void GfxBall::_baseGfxBall() {}
void GfxBall::_killGfxBall() {}

GfxBall::GfxBall(std::string animation_name, int lifetime) : 
    Entity(), 
    _quad(nullptr),
    _animation_name(animation_name),
    _lifetime(lifetime),
    _i(0)
{}

Quad *GfxBall::quad() { return _quad; }