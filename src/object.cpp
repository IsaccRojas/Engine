#include "object.hpp"

Object::Object(glm::vec3 dimensions) : 
    _physdim(dimensions), 
    _physpos(glm::vec3(0.0f)), 
    _collider_ready(false),
    _collision_enabled(false), 
    _collider_id(-1),
    Entity() 
{}
Object::~Object() {
    // try removing from any existing collider
    disableCollision();
}

void Object::_collision(int x) {
    _collisionObject(x);
}

void Object::_initEntity() {
    _initObject();
}
void Object::_baseEntity() {
    _baseObject();
}
void Object::_killEntity() {
    _killObject();

    if (_collider_ready) {
        // remove from collider
        _collider->erase(_collider_id);

        // set collider ID to invalid value
        _collider_id = -1;
    }
}

void Object::_initObject() {}
void Object::_baseObject() {}
void Object::_killObject() {}
void Object::_collisionObject(int x) {}

void Object::objectSetup(Collider *collider) {
    // try removing from any existing collider
    disableCollision();

    _collider = collider;
    _collider_ready = true;
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
        }
    }
}

glm::vec3 Object::getPhysPos() { return _physpos; }
void Object::setPhysPos(glm::vec3 newpos) { _physpos = newpos; }

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
    if (_ids.fillsize() >= _maxcount)
        return -1;
    
    // get a new unique ID
    int id = _ids.push();

    // store object
    _objects[id] = object;
    
    return id;
}

void Collider::erase(int id) {
    if (_ids.at(id))
        _ids.erase_at(id);
}

void Collider::collide() {
    int ids_size = _ids.size();
    for (int i = 0; i < ids_size; i++) {
        if (_ids[i]) {

            for (int j = i + 1; j < ids_size; j++) {
                if (_ids[j]) {

                    if (detectCollision(*_objects[i], *_objects[j])) {
                        _objects[i]->_collision(j);
                        _objects[j]->_collision(i);
                    }

                }
            }
        }
    }
}

// only detects 2D collision for now
bool Collider::detectCollision(const Object &obj1, const Object &obj2) {
    const glm::vec3 &physpos1 = obj1._physpos;
    const glm::vec3 &physdim1 = obj1._physdim;
    const glm::vec3 &physpos2 = obj2._physpos;
    const glm::vec3 &physdim2 = obj2._physdim;
    return 
        (glm::abs(physpos1.x - physpos2.x) * 2 < (physdim1.x + physdim2.x)) 
        && (glm::abs(physpos1.y - physpos2.y) * 2 < (physdim1.y + physdim2.y));
}