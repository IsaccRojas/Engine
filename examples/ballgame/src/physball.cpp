#include "physball.hpp"

void PhysBall::_initEntity() {
    // create and set quad and box
    _quad_off = executor().glenv().genQuad(transform.pos, transform.scale, glm::vec4(1.0f), 0.0f, glm::vec3(0.0f), glm::vec2(0.0f), GLE_ELLIPSE);
    _quad = executor().glenv().getQuad(_quad_off);
    _sphere = executor().spherespace().push(transform, glm::vec3(0.0f), nullptr);
    _sphere->radius = transform.scale.x / 2.0f;

    // set animation and filter if they are named
    if (_animation_name != "")
        _quad->animationstate().setAnimation(&executor().animations()[_animation_name]);
    if (_filter_name != "")
        _sphere->filterstate().setFilter(&executor().filters()[_filter_name]);

    _initPhysBall();
}

void PhysBall::_baseEntity() {
    _basePhysBall();

    // update transform with velocity and set sphere transform to be equal to Script
    transform.pos += vel;
    _sphere->transform = transform;
    _sphere->radius = transform.scale.x / 2.0f;

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

void PhysBall::_killEntity() {
    executor().glenv().remove(_quad_off);
    executor().spherespace().erase(_sphere);

    _killPhysBall();
}

void PhysBall::_initPhysBall() {}
void PhysBall::_basePhysBall() {}
void PhysBall::_killPhysBall() {}

PhysBall::PhysBall(std::string animation_name, std::string filter_name) : 
    Entity(), 
    _quad(nullptr),
    _sphere(nullptr),
    _animation_name(animation_name),
    _filter_name(filter_name),
    vel(glm::vec3(0.0f))
{}

Quad *PhysBall::quad() { return _quad; }
Sphere *PhysBall::sphere() { return _sphere; }