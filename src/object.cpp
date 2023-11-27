#include "object.hpp"

Object::Object() : 
    Entity(),
    _collider(nullptr),
    _collider_id(-1),
    _collider_ready(false),
    _collision_prevpos(glm::vec3(0.0f)),
    _collision_enabled(false),
    _collision_correction(false),
    _collision_elastic(false),
    _physpos(glm::vec3(0.0f)),
    _physvel(glm::vec3(0.0f)),
    _physdim(glm::vec3(0.0f)),
    _physorient(0.0f),
    _objectmanager(nullptr)
{}
Object::~Object() {
    // try removing from any existing collider
    objectResetCollider();
}

void Object::_initEntity() {
    _initObject();
}
void Object::_baseEntity() {
    // store previous position for use with collider
    _collision_prevpos = _physpos;
    _physpos = _physpos + _physvel;
    _baseObject();
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

bool Object::hasCollisionEnabled() {
    return _collision_enabled;
}

bool Object::getCorrection() { return _collision_correction; }
void Object::setCorrection(bool correction) { _collision_correction = correction; }
bool Object::getElastic() { return _collision_elastic; }
void Object::setElastic(bool elastic) { _collision_elastic = elastic; }
glm::vec3 Object::getPhysPos() { return _physpos; }
void Object::setPhysPos(glm::vec3 newpos) { _physpos = newpos; }
glm::vec3 Object::getPhysVel() { return _physvel; }
void Object::setPhysVel(glm::vec3 newvel) { _physvel = newvel; }
glm::vec3 Object::getPhysDim() { return _physdim; }
void Object::setPhysDim(glm::vec3 newdim) { _physdim = newdim; }
float Object::getPhysOrient() { return _physorient; }
void Object::setPhysOrient(float neworient) { _physorient = neworient; }
glm::vec3 Object::getPrevPos() { return _collision_prevpos; }

ObjectManager *Object::getManager() { return _objectmanager; }

// --------------------------------------------------------------------------------------------------------------------------

Collider::Collider(unsigned maxcount) :
    _objects(maxcount, nullptr), 
    _maxcount(maxcount)
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

    // perform pair-wise collision detection
    for (int i = 0; i < ids_size; i++) {
        if (_ids.at(i)) {

            for (int j = i + 1; j < ids_size; j++) {
                if (_ids.at(j)) {
                    
                    // detect and handle collision
                    collisionAABB(_objects[i], _objects[j]);

                }
            }
        }
    }
}

// only detects 2D collision for now
void Collider::collisionAABB(Object *obj1, Object *obj2) {
    glm::vec3 physpos1_curr = obj1->getPhysPos();
    glm::vec3 physpos1_prev = obj1->getPrevPos();
    glm::vec3 physdim1 = obj1->getPhysDim();
    glm::vec3 physpos2_curr = obj2->getPhysPos();
    glm::vec3 physpos2_prev = obj2->getPrevPos();
    glm::vec3 physdim2 = obj2->getPhysDim();
    bool obj1_correct = obj1->getCorrection();
    bool obj2_correct = obj2->getCorrection();

    // get current collision
    bool coll_hor_cur = glm::abs(physpos1_curr.x - physpos2_curr.x) * 2 < (physdim1.x + physdim2.x);
    bool coll_ver_cur = glm::abs(physpos1_curr.y - physpos2_curr.y) * 2 < (physdim1.y + physdim2.y);

    if (coll_hor_cur && coll_ver_cur) {
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

        // run collision handlers
        obj1->collide(obj2);
        obj2->collide(obj1);
    }
}

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

// --------------------------------------------------------------------------------------------------------------------------

ObjectManager::ObjectManager(int maxcount) : EntityManager(maxcount), _collider(nullptr) {
    for (int i = 0; i < maxcount; i++)
        _objectvalues.push_back(ObjectValues{nullptr});
}
ObjectManager::~ObjectManager() {}

void ObjectManager::_objectSetup(Object *object, ObjectType &objecttype, EntityType &entitytype, ScriptType &scripttype, int id) {
    _entitySetup(object, entitytype, scripttype, id);

    if (objecttype._force_objectsetup)
        if (_collider)
            object->objectSetup(_collider);

    object->_objectmanager = this;
}

void ObjectManager::_objectRemoval(ObjectValues &objectvalues, EntityValues &entityvalues, ScriptValues &scriptvalues) {
    _entityRemoval(entityvalues, scriptvalues);
    if (objectvalues._object_ref)
        objectvalues._object_ref->objectResetCollider();
}

bool ObjectManager::hasObject(const char *objectname) { return !(_objecttypes.find(objectname) == _objecttypes.end()); }

Object *ObjectManager::getObject(int id) {
    if (id >= 0 && _ids.at(id))
        return _objectvalues[id]._object_ref;
    else
        return nullptr;
}

int ObjectManager::spawnScript(const char *scriptname) {
    int id = EntityManager::spawnScript(scriptname);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

int ObjectManager::spawnEntity(const char *entityname) {
    int id = EntityManager::spawnEntity(entityname);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

int ObjectManager::spawnObject(const char *objectname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in EntityManager " << this << std::endl;
        return -1;
    }

    // get type information
    ScriptType &scripttype = _scripttypes[objectname];
    EntityType &entitytype = _entitytypes[objectname];
    ObjectType &objecttype = _objecttypes[objectname];

    // push to internal storage
    Object *object = objecttype._allocator();
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(object);
    _objectvalues[id] = ObjectValues{object};
    _entityvalues[id] = EntityValues{object};
    _scriptvalues[id] = ScriptValues{id, objectname, object};
    
    // set up entity
    _objectSetup(object, objecttype, entitytype, scripttype, id);
    
    return id;
}

void ObjectManager::addObject(std::function<Object*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name, bool force_objectsetup) {
    if (!hasObject(name) && !hasEntity(name) && !hasScript(name)) {
        _objecttypes[name] = ObjectType{
            force_objectsetup,
            allocator
        };
        addEntity(allocator, name, type, force_scriptsetup, force_enqueue, force_removeonkill, force_entitysetup, animation_name);
    }
}

void ObjectManager::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Manager " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Manager " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        // get info
        ScriptValues &scriptvalues = _scriptvalues[id];
        EntityValues &entityvalues = _entityvalues[id];
        ObjectValues &objectvalues = _objectvalues[id];

        // remove from object-related, entity-related and script-related systems
        _objectRemoval(objectvalues, entityvalues, scriptvalues);
    
        _ids.erase_at(id);
    }
}

void ObjectManager::setCollider(Collider *collider) { if (!_collider) _collider = collider; }

glm::vec3 random_angle(glm::vec3 v, float deg) {
    if (deg == 0.0f)
        return v;
    return glm::rotate(v, glm::radians((-1.0f * deg) + float(rand() % int(deg * 2.0f))), glm::vec3(0.0f, 0.0f, 1.0f));
}