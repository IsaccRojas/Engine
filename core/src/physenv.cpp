#include "../include/physenv.hpp"

// _______________________________________ Box _______________________________________

Box::Box(glm::vec3 position, glm::vec3 velocity, glm::vec3 dimensions, std::function<void(Box*)> callback) :
    _prevpos(position),
    _callback(callback),
    pos(position),
    dim(dimensions),
    vel(velocity)
{}

Box::Box(const Box &other) {
    _prevpos = other._prevpos;
    _callback = other._callback;
    pos = other.pos;
    dim = other.dim;
    vel = other.vel;
}

Box::Box() :
    _prevpos(glm::vec3(0.0f)),
    _callback(nullptr),
    pos(glm::vec3(0.0f)),
    dim(glm::vec3(0.0f)),
    vel(glm::vec3(0.0f))
{}

Box& Box::operator=(const Box &other) {
    _prevpos = other._prevpos;
    _callback = other._callback;
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
    if (i >= 0 && _ids[i])
        return &_boxes[i];
    return nullptr;
}

void PhysEnv::step() {
    for (unsigned i = 0; i < _ids.size(); i++)
        // only try calling step on index i if it is an active ID in _ids
        if (_ids[i])
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
    //bool obj1_correct = obj1->getCorrection();
    //bool obj2_correct = obj2->getCorrection();

    // get current collision
    bool coll_hor_cur = glm::abs(pos1.x - pos2.x) * 2 < (dim1.x + dim2.x);
    bool coll_ver_cur = glm::abs(pos1.y - pos2.y) * 2 < (dim1.y + dim2.y);

    if (coll_hor_cur && coll_ver_cur) {
        /*
        if (obj1_correct || obj2_correct) {
            // get past collision
            bool coll_hor_prev = glm::abs(physpos1_prev.x - physpos2_prev.x) * 2 < (physdim1.x + physdim2.x);
            //bool coll_ver_prev = glm::abs(physpos1_prev.y - physpos2_prev.y) * 2 < (physdim1.y + physdim2.y);

            if (coll_hor_prev) {
                // if colliding horizontally before, correct objects along x coordinate based on previous position
                bool obj1_front_obj2 = physpos1_prev.x >= physpos2_prev.x;
                if (obj1_correct)
                    obj1->setPhysPos(
                        obj1->getPhysPos() +
                        (glm::vec3(physpos2_curr.x + physdim2.x + physdim1.x, 0.0f, 0.0f) * ((obj1_front_obj2) ? 1.0f : -1.0f))
                    );
                if (obj2_correct)
                    obj2->setPhysPos(
                        obj2->getPhysPos() +
                        (glm::vec3(physpos1_curr.x + physdim1.x + physdim2.x, 0.0f, 0.0f) * ((!obj1_front_obj2) ? 1.0f : -1.0f))
                    );
            // if no horizontal collision, assume vertical
            } else {
                // if colliding horizontally before, correct objects along x coordinate based on previous position
                bool obj1_above_obj2 = physpos1_prev.y >= physpos2_prev.y;
                if (obj1_correct)
                    obj1->setPhysPos(
                        obj1->getPhysPos() +
                        (glm::vec3(0.0f, physpos2_curr.y + physdim2.y + physdim1.y, 0.0f) * ((obj1_above_obj2) ? 1.0f : -1.0f))
                    );
                if (obj2_correct)
                    obj2->setPhysPos(
                        obj2->getPhysPos() +
                        (glm::vec3(0.0f, physpos1_curr.y + physdim1.y + physdim2.y, 0.0f) * ((!obj1_above_obj2) ? 1.0f : -1.0f))
                    );
            }
        }
        */

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