#include "object.hpp"

Object::Object() : 
    _physdim(glm::vec3(0.0f)), 
    _physpos(glm::vec3(0.0f)),
    _physvel(glm::vec3(0.0f)),
    _collider_ready(false),
    _collision_enabled(false), 
    _collider_id(-1),
    Entity() 
{}
Object::~Object() {
    // try removing from any existing collider
    objectResetCollider();
}

void Object::_initEntity() {
    _initObject();
}
void Object::_baseEntity() {
    _baseObject();

    _physpos = _physpos + _physvel;
}
void Object::_killEntity() {
    _killObject();
}

void Object::_initObject() {}
void Object::_baseObject() {}
void Object::_killObject() {}
void Object::_collisionObject(Object *other) {}

void Object::objectSetup(Collider *collider) {
    // try removing from any existing collider
    objectResetCollider();

    _collider = collider;
    _collider_ready = true;
}

void Object::objectResetCollider() {
    if (_collider_ready)
        if (_collider_id >= 0)
            _collider->erase(_collider_id);

    _collider = nullptr;
    _collider_ready = false;
    _collider_id = -1;
}

void Object::collide(Object *other) {
    _collisionObject(other);
}

void Object::enableCollision() {
    if (_collider_ready) {
        if (_collider_id < 0) {
            // add to collider
            _collider_id = _collider->push(this);

            // update flag if successfully pushed
            if (_collider_id >= 0)
                _collision_enabled = true;
        }
    }
}

void Object::disableCollision() {
    if (_collider_ready) {
        if (_collider_id >= 0) {
            // remove from collider
            _collider->erase(_collider_id);

            // set collider ID to invalid value
            _collider_id = -1;
            _collision_enabled = false;

            // keep collider reference and ready flag
        }
    }
}

glm::vec3 Object::getPhysPos() { return _physpos; }
void Object::setPhysPos(glm::vec3 newpos) { _physpos = newpos; }
glm::vec3 Object::getPhysVel() { return _physvel; }
void Object::setPhysVel(glm::vec3 newvel) { _physvel = newvel; }
glm::vec3 Object::getPhysDim() { return _physdim; }
void Object::setPhysDim(glm::vec3 newdim) { _physdim = newdim; }

bool Object::hasCollisionEnabled() {
    return _collision_enabled;
}

// --------------------------------------------------------------------------------------------------------------------------

Collider::Collider(int maxcount) : 
    _maxcount(maxcount),
    _objects(maxcount, nullptr)
{}
Collider::~Collider() {}

int Collider::push(Object *object) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Collider " << this << std::endl;
        return -1;
    }
    
    // get a new unique ID
    int id = _ids.push();

    // store object
    _objects[id] = object;
    
    return id;
}

void Collider::erase(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Collider " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Collider " << this << std::endl;
        return;
    }

    if (_ids.at(id))
        _ids.erase_at(id);
}

void Collider::collide() {
    int ids_size = _ids.size();
    for (int i = 0; i < ids_size; i++) {
        if (_ids.at(i)) {

            for (int j = i + 1; j < ids_size; j++) {
                if (_ids.at(j)) {

                    if (detectCollision(*_objects[i], *_objects[j])) {
                        _objects[i]->collide(_objects[j]);
                        _objects[j]->collide(_objects[i]);
                    }

                }
            }
        }
    }
}

// only detects 2D collision for now
bool Collider::detectCollision(Object &obj1, Object &obj2) {
    const glm::vec3 &physpos1 = obj1.getPhysPos();
    const glm::vec3 &physdim1 = obj1.getPhysDim();
    const glm::vec3 &physpos2 = obj2.getPhysPos();
    const glm::vec3 &physdim2 = obj2.getPhysDim();
    return 
        (glm::abs(physpos1.x - physpos2.x) * 2 < (physdim1.x + physdim2.x)) 
        && (glm::abs(physpos1.y - physpos2.y) * 2 < (physdim1.y + physdim2.y));
}