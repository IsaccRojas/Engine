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

ObjectManager *Object::getManager() { return _objectmanager; }

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