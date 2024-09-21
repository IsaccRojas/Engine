#include "physball.hpp"

void PhysBall::_initEntity() {
    // create and set quad and box
    _quad = executor().glenv().getQuad(
        executor().glenv().genQuad(transform().pos, transform().scale, glm::vec4(1.0f), glm::vec3(0.0f), glm::vec2(0.0f), GLE_ELLIPSE)
    );
    _box = executor().physenv().push(transform(), glm::vec3(0.0f), nullptr);

    // set animation and filter if they are named
    if (_animation_name != "")
        _quad->setAnim(&executor().animations()[_animation_name]);
    if (_filter_name != "")
        _box->setFilter(&executor().filters()[_filter_name]);


    _initPhysBall();
}

void PhysBall::_baseEntity() {
    _basePhysBall();

    // update quad and box to match Script transform (except for z-coordinate), and step animation
    _box->transform = transform();
    _quad->bv_pos.v = glm::vec3(transform().pos.x, transform().pos.y, _quad->bv_pos.v.z);

    _quad->stepAnim();

    // only queue if not set to be killed
    if (!getKillEnqueued())
        enqueueExec(getLastExecQueue());
}

void PhysBall::_killEntity() {
    _quad->bv_scale.v = glm::vec3(0.0f);
    _box->transform.scale = glm::vec3(0.0f);
    _killPhysBall();
}

void PhysBall::_initPhysBall() {}
void PhysBall::_basePhysBall() {}
void PhysBall::_killPhysBall() {}

PhysBall::PhysBall(std::string animation_name, std::string filter_name) : 
    Entity(), 
    _quad(nullptr),
    _box(nullptr),
    _animation_name(animation_name),
    _filter_name(filter_name)
{}

Quad *PhysBall::quad() { return _quad; }
Box *PhysBall::box() { return _box; }