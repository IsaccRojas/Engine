#include "../include/object.hpp"

Object::Object() : 
    Entity(),
    _physenv(nullptr),
    _filter(nullptr),
    _box(nullptr),
    _box_id(-1),
    _collision_correction(false),
    _collision_elastic(false),
    _physenv_ready(false),
    _box_ready(false),
    _objectmanager(nullptr)
{}
Object::~Object() {
    // try removing existing box
    removeBox();
}

void Object::_initEntity() {
    if (_physenv_ready)
        _initObject();
}
void Object::_baseEntity() {
    if (_physenv_ready) {
        if (_box_ready) {
            // advance box in time
            _box->step();
        }

        _baseObject();
    }
}
void Object::_killEntity() {
    if (_physenv_ready)
        _killObject();
}

void Object::_initObject() {}
void Object::_baseObject() {}
void Object::_killObject() {}
void Object::_collisionObject(Box *box) {}

void Object::objectSetup(PhysEnv* physenv, Filter *filter) {
    // try removing existing box
    removeBox();

    _physenv = physenv;
    _filter = filter;
    _physenv_ready = true;

    // generate new box
    genBox(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));
    _box->setFilter(_filter);
}

bool Object::getCorrection() { return _collision_correction; }
void Object::setCorrection(bool correction) { _collision_correction = correction; }
bool Object::getElastic() { return _collision_elastic; }
void Object::setElastic(bool elastic) { _collision_elastic = elastic; }

void Object::genBox(glm::vec3 position, glm::vec3 dimensions, glm::vec3 velocity) {
    if (_physenv_ready) {
        // erase existing box
        if (_box_ready)
            removeBox();

        // get box data
        _box_id = _physenv->genBox(position, dimensions, velocity, std::bind(&(Object::_collisionObject), this, std::placeholders::_1));
        
        // if successful, retrieve box
        if (_box_id >= 0) {
            _box = _physenv->get(_box_id);
            _box_ready = true;
        }
    }
}

void Object::removeBox() {
    if (_box_id >= 0) {
        _physenv->remove(_box_id);
        _box_id = -1;

        _box_ready = false;
    }
}

Box *Object::getBox() { 
    if (_box_ready)
        return _box;
    else
        return nullptr;
}

ObjectManager *Object::getManager() { return _objectmanager; }

// --------------------------------------------------------------------------------------------------------------------------

ObjectManager::ObjectManager(int maxcount) : EntityManager(maxcount), _physenv(nullptr), _filters(nullptr) {
    for (int i = 0; i < maxcount; i++)
        _objectvalues.push_back(ObjectValues{nullptr});
}
ObjectManager::~ObjectManager() {}

void ObjectManager::_objectSetup(Object *object, ObjectType &objecttype, EntityType &entitytype, ScriptType &scripttype, int id) {
    _entitySetup(object, entitytype, scripttype, id);

    if (objecttype._force_objectsetup)
        if (_physenv)
            object->objectSetup(_physenv, &(*_filters)[objecttype._filter_name]);

    object->_objectmanager = this;
}

void ObjectManager::_objectRemoval(ObjectValues &objectvalues, EntityValues &entityvalues, ScriptValues &scriptvalues) {
    _entityRemoval(entityvalues, scriptvalues);
    if (objectvalues._object_ref)
        objectvalues._object_ref->removeBox();
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
    if (_ids.fillSize() >= _maxcount) {
        std::cerr << "WARN: limit reached in ObjectManager " << this << std::endl;
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
    
    // set up object
    _objectSetup(object, objecttype, entitytype, scripttype, id);
    
    return id;
}

void ObjectManager::addObject(std::function<Object*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name, bool force_objectsetup, const char *filter_name) {
    if (!hasObject(name) && !hasEntity(name) && !hasScript(name)) {
        _objecttypes[name] = ObjectType{
            force_objectsetup,
            filter_name,
            allocator
        };
        addEntity(allocator, name, type, force_scriptsetup, force_enqueue, force_removeonkill, force_entitysetup, animation_name);
    }
}

void ObjectManager::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from ObjectManager " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty ObjectManager " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        // get info
        ScriptValues &scriptvalues = _scriptvalues[id];
        EntityValues &entityvalues = _entityvalues[id];
        ObjectValues &objectvalues = _objectvalues[id];

        // remove from object-related, entity-related and script-related systems
        _objectRemoval(objectvalues, entityvalues, scriptvalues);
    
        _ids.remove(id);
    }
}

void ObjectManager::setPhysEnv(PhysEnv *physenv) { if (!_physenv) _physenv = physenv; }
void ObjectManager::setFilters(std::unordered_map<std::string, Filter> *filters) { if (!_filters) _filters = filters; }