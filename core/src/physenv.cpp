#include "../include/physenv.hpp"

// _______________________________________ Box _______________________________________

Box::Box(Transform transformation, glm::vec3 velocity, std::function<void(Box*)> callback) :
    _physenv(nullptr),
    _collided(false),
    _prev_pos(transformation.pos),
    _callback(callback),
    transform(transformation),
    vel(velocity),
    mass(1.0f)
{}

Box::Box() :
    _physenv(nullptr),
    _collided(false),
    _prev_pos(glm::vec3(0.0f)),
    _callback(nullptr),
    transform(Transform{glm::vec3(0.0f), glm::vec3(0.0f)}),
    vel(glm::vec3(0.0f)),
    mass(1.0f)
{}

Box::~Box() { /* automatic destruction is fine */ }

void Box::setCallback(std::function<void(Box*)> callback) {
    _callback = callback;
}

void Box::step() {
    _prev_pos = transform.pos;
    transform.pos = transform.pos + vel;
}

void Box::collide(Box *box) {
    if (_callback)
        _callback(box);
}

void Box::setFilter(Filter *filter) {
    _filter_state.setFilter(filter);
}

FilterState& Box::filterstate() {
    return _filter_state;
}

glm::vec3& Box::prevpos() {
    return _prev_pos;
}

bool Box::getCollided() { return _collided; }

// _______________________________________ PhysEnv _______________________________________

PhysEnv::PhysEnv() {}
PhysEnv::~PhysEnv() { /* automatic destruction is fine */ }

Box *PhysEnv::push(Transform transf, glm::vec3 vel, std::function<void(Box*)> callback) {
    _boxes.push_back(Box{transf, vel, callback});
    auto iter = _boxes.end();
    iter--;

    // assign iterator position and this reference to Box
    Box *b = &(*iter);
    b->_physenv = this;
    b->_this_iter = iter;

    return b;
}

void PhysEnv::erase(Box *box) {
    if (box->_physenv != this)
        throw std::runtime_error("Attempt to erase Box from PhysEnv that does not own it");
    
    _boxes.erase(box->_this_iter);
}

void PhysEnv::unsetCollidedFlags() {
    for (auto &box : _boxes)
        box._collided = false;
}

void PhysEnv::detectCollision() {
    // perform pair-wise collision detection
    for (auto iter1 = _boxes.begin(); iter1 != _boxes.end(); iter1++) {

        // get box and skip if zeroed out
        Box &box1 = *iter1;
        if (box1.transform.scale == glm::vec3(0.0f))
            continue;

        auto iter2 = iter1;
        iter2++;
        for (;iter2 != _boxes.end(); iter2++) {

            Box &box2 = *iter2;
            if (box2.transform.scale == glm::vec3(0.0f))
                continue;

            // test filters against each other's IDs
            bool f1 = box1.filterstate().hasFilter();
            bool f2 = box2.filterstate().hasFilter();
            
            // if both have a filter, collide if both pass
            // if neither have a filter, collide
            // if only one has a filter, skip
            if (f1 != f2)
                continue;
            if (
                (!f1 && !f2) ||
                    (box1.filterstate().pass(box2.filterstate().id()) &&
                    box2.filterstate().pass(box1.filterstate().id()))
            )
            
            // detect and handle collision
            collisionAABB(box1, box2);

        }

    }
}

void PhysEnv::step() {
    for (auto &box : _boxes)
        box.step();
}

bool PhysEnv::hasBox(Box *box) {
    return (box->_physenv == this);
}

void PhysEnv::clear() {
    _boxes.clear();
}

// only detects 2D collision for now
void PhysEnv::collisionAABB(Box &box1, Box &box2) {
    glm::vec3 &pos1 = box1.transform.pos;
    glm::vec3 &dim1 = box1.transform.scale;
    glm::vec3 &pos2 = box2.transform.pos;
    glm::vec3 &dim2 = box2.transform.scale;

    // get current collision
    float coll_hor_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
    float coll_ver_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);

    if (coll_hor_space < 0.0f && coll_ver_space < 0.0f) {
        // set flags
        box1._collided = true;
        box2._collided = true;

        // run collision handlers
        box1.collide(&box2);
        box2.collide(&box1);
    }
}

glm::vec3 random_angle(glm::vec3 v, float deg) {
    if (deg == 0.0f)
        return v;
    return glm::rotate(v, glm::radians((-1.0f * deg) + float(rand() % int(deg * 2.0f))), glm::vec3(0.0f, 0.0f, 1.0f));
}

/*

// only detects 2D collision for now
float Collider::collisionDistance(Object &obj1, Object &obj2) {
    const glm::vec3 &physpos1 = obj1.getPhysPos();
    const glm::vec3 &physdim1 = obj1.getPhysDim();
    const glm::vec3 &physpos2 = obj2.getPhysPos();
    const glm::vec3 &physdim2 = obj2.getPhysDim();
    glm::vec3 vdiff;
    float angle1, angle2, side1, side2, overlap1, overlap2;

    // get difference of positions
    vdiff = glm::abs(physpos2 - physpos1);

    // get whether collision was horizontal or not for object 1
    if ((vdiff.x / vdiff.y) >= (physdim1.x / physdim1.y)) {
        side1 = physdim1.x / 2.0f;
        angle1 = glm::atan(vdiff.y / vdiff.x);
    } else {
        side1 = physdim1.y / 2.0f;
        angle1 = glm::atan(vdiff.x / vdiff.y);
    }

    // get whether collision was horizontal or not for object 2
    if ((vdiff.x / vdiff.y) >= (physdim2.x / physdim2.y)) {
        side2 = physdim2.x / 2.0f;
        angle2 = glm::atan(vdiff.y / vdiff.x);
    } else {
        side2 = physdim2.y / 2.0f;
        angle2 = glm::atan(vdiff.x / vdiff.y);
    }

    // get overlaps
    overlap1 = side1 / glm::cos(angle1);
    overlap2 = side2 / glm::cos(angle2);

    return (glm::distance(physpos1, physpos2) - overlap1) - overlap2;
}

*/