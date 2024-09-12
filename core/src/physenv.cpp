#include "../include/physenv.hpp"

// _______________________________________ Box _______________________________________

Box::Box(glm::vec3 position, glm::vec3 velocity, glm::vec3 dimensions, std::function<void(Box*)> callback) :
    _collided(false),
    _prev_pos(position),
    _callback(callback),
    pos(position),
    dim(dimensions),
    vel(velocity),
    mass(1.0f)
{}

Box::Box() :
    _collided(false),
    _prev_pos(glm::vec3(0.0f)),
    _callback(nullptr),
    pos(glm::vec3(0.0f)),
    dim(glm::vec3(0.0f)),
    vel(glm::vec3(0.0f)),
    mass(1.0f)
{}

Box::~Box() { /* automatic destruction is fine */ }

void Box::setCallback(std::function<void(Box*)> callback) {
    _callback = callback;
}

void Box::step() {
    _prev_pos = pos;
    pos = pos + vel;
}

void Box::collide(Box *box) {
    _callback(box);
}

void Box::setFilter(Filter *filter) {
    _filter_state.setFilter(filter);
}

FilterState& Box::getFilterState() {
    return _filter_state;
}

glm::vec3& Box::getPrevPos() {
    return _prev_pos;
}

bool Box::getCollided() { return _collided; }

// _______________________________________ PhysEnv _______________________________________

PhysEnv::PhysEnv(unsigned max_count) : _initialized(false) {
    init(max_count);
}
PhysEnv::PhysEnv() : _max_count(0), _count(0), _initialized(false) {}
PhysEnv::~PhysEnv() { /* automatic destruction is fine */ }

void PhysEnv::init(unsigned max_count) {
    if (_initialized)
        throw InitializedException();
    
    _boxes = std::vector<Box>(max_count, Box());
    _max_count = max_count;
    _count = 0;
    _initialized = true;
}

void PhysEnv::uninit() {
    if (!_initialized)
        return;
    
    _ids.clear();
    _boxes.clear();
    _max_count = 0;
    _count = 0;
    _initialized = false;
}

unsigned PhysEnv::genBox(glm::vec3 pos, glm::vec3 dim, glm::vec3 vel, std::function<void(Box*)> callback) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_count >= _max_count)
        throw CountLimitException();

    // get a new unique ID
    unsigned id = _ids.push();
    
    // generate box by providing parameters and use id to specify an offset into them
    _boxes[id] = Box{pos, dim, vel, callback};

    _count++;
    return id;
}

void PhysEnv::remove(unsigned id) {
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    // call _ids to make the ID usable again
    _ids.remove(id);

    _count--;
}

void PhysEnv::unsetCollidedFlags() {
    for (unsigned i = 0; i < _ids.size(); i++)
        if (_ids.at(i))
            _boxes[i]._collided = false;
}

void PhysEnv::detectCollision() {
    unsigned ids_size = _ids.size();

    // perform pair-wise collision detection
    for (unsigned i = 0; i < ids_size; i++) {
        if (_ids.at(i)) {
            // get box and skip if zeroed out
            Box &box1 = _boxes[i];
            if (box1.dim == glm::vec3(0.0f))
                continue;

            for (unsigned j = i + 1; j < ids_size; j++) {
                if (_ids.at(j)) {
                    Box &box2 = _boxes[j];
                    if (box2.dim == glm::vec3(0.0f))
                        continue;

                    // test filters against each other's IDs
                    bool f1 = box1.getFilterState().hasFilter();
                    bool f2 = box2.getFilterState().hasFilter();
                    
                    // if both have a filter, collide if both pass
                    // if neither have a filter, collide
                    // if only one has a filter, skip
                    if (f1 != f2)
                        continue;
                    if (
                        (!f1 && !f2) ||
                            (box1.getFilterState().pass(box2.getFilterState().id()) &&
                            box2.getFilterState().pass(box1.getFilterState().id()))
                    )
                        // detect and handle collision
                        collisionAABB(box1, box2);

                }
            }

        }
    }
}

void PhysEnv::step() {
    for (unsigned i = 0; i < _ids.size(); i++)
        // only try calling step on index i if it is an active ID in _ids
        if (_ids.at(i))
            _boxes[i].step();
}

Box *PhysEnv::getBox(unsigned id) {
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        return &(_boxes[id]);
    
    throw InactiveIDException();
}

std::vector<unsigned> PhysEnv::getIDs() { return _ids.getUsed(); }

bool PhysEnv::hasID(unsigned id) { return _ids.at(id); }

bool PhysEnv::getInitialized() { return _initialized; }

// only detects 2D collision for now
void PhysEnv::collisionAABB(Box &box1, Box &box2) {
    glm::vec3 &pos1 = box1.pos;
    glm::vec3 &dim1 = box1.dim;
    glm::vec3 &pos2 = box2.pos;
    glm::vec3 &dim2 = box2.dim;

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