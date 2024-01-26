#include "../include/physenv.hpp"

// _______________________________________ Box _______________________________________

Box::Box(glm::vec3 position, glm::vec3 velocity, glm::vec3 dimensions, std::function<void(Box*)> callback) :
    _prevpos(position),
    _callback(callback),
    _collision_correction(false),
    pos(position),
    dim(dimensions),
    vel(velocity)
{}

Box::Box(const Box &other) {
    _prevpos = other._prevpos;
    _callback = other._callback;
    _collision_correction = other._collision_correction;
    pos = other.pos;
    dim = other.dim;
    vel = other.vel;
}

Box::Box() :
    _prevpos(glm::vec3(0.0f)),
    _callback(nullptr),
    _collision_correction(false),
    pos(glm::vec3(0.0f)),
    dim(glm::vec3(0.0f)),
    vel(glm::vec3(0.0f))
{}

Box& Box::operator=(const Box &other) {
    _prevpos = other._prevpos;
    _callback = other._callback;
    _collision_correction = other._collision_correction;
    pos = other.pos;
    dim = other.dim;
    vel = other.vel;

    return *this;
}

Box::~Box() {}

void Box::setCallback(std::function<void(Box*)> callback) {
    _callback = callback;
}

void Box::step() {
    _prevpos = pos;
    pos = pos + vel;
}

void Box::collide(Box *box) {
    _callback(box);
}

/* Sets the box's collision filter. */
void Box::setFilter(Filter *filter) {
    _filterstate.setFilter(filter);
}

/* Gets the box's collision filter state. */
FilterState& Box::getFilterState() {
    return _filterstate;
}

/* Sets the box's collision correction flag. */
void Box::setCorrection(bool correction) {
    _collision_correction = correction;
}

/* Gets the box's collision correction flag. */
bool Box::getCorrection() {
    return _collision_correction;
}

/* Gets the box's previous position. */
glm::vec3& Box::getPrevPos() {
    return _prevpos;
}

// _______________________________________ PhysEnv _______________________________________

PhysEnv::PhysEnv(unsigned maxcount) :
    _boxes(maxcount, Box()),
    _maxcount(maxcount)
{}

PhysEnv::~PhysEnv() {}

int PhysEnv::genBox(glm::vec3 pos, glm::vec3 dim, glm::vec3 vel, std::function<void(Box*)> callback) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillSize() >= _maxcount) {
        std::cerr << "WARN: limit reached in PhysEnv " << this << std::endl;
        return -1;
    }

    // get a new unique ID
    int id = _ids.push();
    
    // generate box by providing parameters and use id to specify an offset into them
    _boxes[id] = Box{pos, dim, vel, callback};

    return id;
}

Box *PhysEnv::get(int i) {
    if (i < 0)
        std::cerr << "WARN: attempt to get address with negative value from PhysEnv " << this << std::endl;
    if (i >= 0 && _ids.at(i))
        return &(_boxes[i]);
    return nullptr;
}

void PhysEnv::step() {
    for (unsigned i = 0; i < _ids.size(); i++)
        // only try calling step on index i if it is an active ID in _ids
        if (_ids.at(i))
            _boxes[i].step();
}

int PhysEnv::remove(int id) {
    // if attempting to remove an id from empty system, return -1
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from PhysEnv " << this << std::endl;
        return -1;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty PhysEnv " << this << std::endl;
        return -1;
    }

    // call _ids to make the ID usable again
    _ids.remove(id);

    return 0;
}

void PhysEnv::detectCollision() {
    int ids_size = _ids.size();

    // perform pair-wise collision detection
    for (int i = 0; i < ids_size; i++) {
        if (_ids.at(i)) {

            for (int j = i + 1; j < ids_size; j++) {
                if (_ids.at(j)) {
                    
                    // detect and handle collision
                    if (
                        _boxes[i].getFilterState().pass(_boxes[j].getFilterState().id()) &&
                        _boxes[j].getFilterState().pass(_boxes[i].getFilterState().id())
                    )
                        collisionAABB(_boxes[i], _boxes[j]);

                }
            }
        }
    }
}

std::vector<int> PhysEnv::getids() {
    return _ids.getUsed();
}

// only detects 2D collision for now
void PhysEnv::collisionAABB(Box &box1, Box &box2) {
    glm::vec3 &pos1 = box1.pos;
    glm::vec3 &dim1 = box1.dim;
    glm::vec3 &pos2 = box2.pos;
    glm::vec3 &dim2 = box2.dim;

    // get current collision
    float coll_hor_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
    float coll_ver_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);

    if (coll_hor_space < 0 && coll_ver_space < 0) {  
        bool correct1 = box1.getCorrection();
        bool correct2 = box2.getCorrection();

        // if correction is set, get data on previous positions
        if (correct1 || correct2) {
            glm::vec3 &vel1 = box1.vel;
            glm::vec3 &vel2 = box2.vel;
            glm::vec3 &prevpos1 = box1.getPrevPos();
            glm::vec3 &prevpos2 = box2.getPrevPos();
            
            // get previous collision state and relative positions
            bool coll_hor_prev = glm::abs(prevpos1.x - prevpos2.x) < (dim1.x + dim2.x) / 2.0f;
            bool coll_ver_prev = glm::abs(prevpos1.y - prevpos2.y) < (dim1.y + dim2.y) / 2.0f;
            bool box1_right = prevpos1.x >= prevpos2.x;
            bool box1_above = prevpos1.y >= prevpos2.y;

            // apply corrections
            float vel1_ratio = 0.0f;
            float vel2_ratio = 0.0f;
            if (correct1 && correct2) {
                // get ratio of corrections based on velocities
                vel1_ratio = glm::length(vel1) / (glm::length(vel1) + glm::length(vel2));
                vel2_ratio = glm::length(vel2) / (glm::length(vel1) + glm::length(vel2));
            } else {
                // set one box to get the full correction, and the other to get no correction
                vel1_ratio = correct1;
                vel2_ratio = correct2;
            }

            if (!coll_hor_prev) {
                if (!box1_right) {
                    // move box1 to the left and box2 to the right
                    pos1.x -= glm::abs(coll_hor_space) * vel1_ratio;
                    pos2.x += glm::abs(coll_hor_space) * vel2_ratio;
                } else {
                    // move box1 to the right and box2 to the left
                    pos1.x += glm::abs(coll_hor_space) * vel1_ratio;
                    pos2.x -= glm::abs(coll_hor_space) * vel2_ratio;
                }
            }
            if (!coll_ver_prev) {
                if (!box1_above) {
                    // move box1 down and box2 up
                    pos1.y -= glm::abs(coll_ver_space) * vel1_ratio;
                    pos2.y += glm::abs(coll_ver_space) * vel2_ratio;
                } else {
                    // move box1 up and box2 down
                    pos1.y += glm::abs(coll_ver_space) * vel1_ratio;
                    pos2.y -= glm::abs(coll_ver_space) * vel2_ratio;
                }
            }
        }

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