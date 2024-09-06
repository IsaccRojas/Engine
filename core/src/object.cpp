#include "../include/object.hpp"

// throws if initialized is true
void _checkInitialized(bool initialized) {
    if (initialized)
        throw InitializedException();
}

// throws if initialized is false
void _checkUninitialized(bool initialized) {
    if (!initialized)
        throw UninitializedException();
}

Object::Object(Object &&other) {
    operator=(std::move(other));
}
Object::Object() : 
    Entity(),
    _physenv(nullptr),
    _filter(nullptr),
    _box(nullptr),
    _box_id(-1),
    _objectmanager(nullptr)
{}
Object::~Object() {
    // try removing existing Box
    removeBox();
}

Object &Object::operator=(Object &&other) {
    if (this != &other) {
        Entity::operator=(std::move(other));
        _physenv = other._physenv;
        _filter = other._filter;
        _box = other._box;
        _box_id = other._box_id;
        _objectmanager = other._objectmanager;
        other._physenv = nullptr;
        other._filter = nullptr;
        other._box = nullptr;
        other._box_id = -1;
        other._objectmanager = nullptr;
    }
    return *this;
}

void Object::_initEntity() {
    if (_physenv)
        _initObject();
}
void Object::_baseEntity() {
    if (_physenv)
        _baseObject();
}
void Object::_killEntity() {
    if (_physenv)
        _killObject();
}

void Object::_initObject() {}
void Object::_baseObject() {}
void Object::_killObject() {}
void Object::_collisionObject(Box *box) {}

void Object::objectSetup(PhysEnv* physenv, Filter *filter) {
    if (!physenv)
        throw std::runtime_error("Attempt to setup Object with null PhysEnv reference");
    
    // try removing existing PhysEnv and Filter information
    objectClear();

    _physenv = physenv;
    _filter = filter;
}

void Object::objectClear() {
    // try removing existing Box
    removeBox();
    _physenv = nullptr;
    _filter = nullptr;
}

void Object::genBox(glm::vec3 position, glm::vec3 dimensions, glm::vec3 velocity) {
    if (_physenv) {
        // erase existing Box
        if (_box)
            removeBox();

        // get Box data
        _box_id = _physenv->genBox(position, dimensions, velocity, std::bind(&(Object::_collisionObject), this, std::placeholders::_1));
        _box = _physenv->get(_box_id);
        _box->setFilter(_filter);
    } else
        throw std::runtime_error("Attempt to generate Quad with null PhysEnv reference");
}

void Object::removeBox() {
    if (_physenv && _box) {
        _physenv->remove(_box_id);

        _box = nullptr;
        _box_id = -1;
    }
}

Box *Object::getBox() { 
    return _box;
}

ObjectManager *Object::getManager() { return _objectmanager; }

// --------------------------------------------------------------------------------------------------------------------------

ObjectManager::ObjectManager(unsigned max_count, PhysEnv *physenv, unordered_map_string_Filter_t *filters, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor) : EntityManager() {
    init(max_count, physenv, filters, glenv, animations, executor);
}
ObjectManager::ObjectManager(ObjectManager &&other) {
    operator=(std::move(other));
}
ObjectManager::ObjectManager() : EntityManager(), _physenv(nullptr), _filters(nullptr) {}
ObjectManager::~ObjectManager() { /* automatic destruction is fine */ }

ObjectManager& ObjectManager::operator=(ObjectManager &&other) {
    if (this != &other) {
        EntityManager::operator=(std::move(other));
        _objectinfos = other._objectinfos;
        _objectvalues = other._objectvalues;
        _physenv = other._physenv;
        _filters = other._filters;
        other._objectManagerUninit();
    }
    return *this;
}

void ObjectManager::_objectManagerInit(unsigned max_count, PhysEnv *physenv, unordered_map_string_Filter_t *filters) {
    _physenv = physenv;
    _filters = filters;
    for (unsigned i = 0; i < max_count; i++)
        _objectvalues.push_back(ObjectValues{nullptr});
}

void ObjectManager::_objectManagerUninit() {
    _objectinfos.clear();
    _objectvalues.clear();
    _physenv = nullptr;
    _filters = nullptr;
}

void ObjectManager::_objectSetup(Object *object, ObjectInfo &objectinfo, EntityInfo &entityinfo, ScriptInfo &scriptinfo, unsigned id) {
    _entitySetup(object, entityinfo, scriptinfo, id);

    object->objectSetup(_physenv, &(*_filters)[objectinfo._filter_name]);
    object->genBox(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));

    object->_objectmanager = this;
}

void ObjectManager::_objectRemoval(ObjectValues &objectvalues, EntityValues &entityvalues, ScriptValues &scriptvalues) {
    _entityRemoval(entityvalues, scriptvalues);

    // accessing ref after entity-level removal is fine because deletion doesn't actually take place
    objectvalues._object_ref->removeBox();

    objectvalues = ObjectValues{nullptr};
}

void ObjectManager::init(unsigned max_count, PhysEnv *physenv, unordered_map_string_Filter_t *filters, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor) {
    EntityManager::init(max_count, glenv, animations, executor);
    _objectManagerInit(max_count, physenv, filters);
}

void ObjectManager::uninit() {
    EntityManager::uninit();
    _objectManagerUninit();
}

bool ObjectManager::hasObject(const char *object_name) { return !(_objectinfos.find(object_name) == _objectinfos.end()); }

Object *ObjectManager::getObject(unsigned id) {
    _checkUninitialized(_initialized);

    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");

    return _objectvalues[id]._object_ref;

    throw InactiveIDException();
}

unsigned ObjectManager::spawnScript(const char *script_name) {
    unsigned id = EntityManager::spawnScript(script_name);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

unsigned ObjectManager::spawnEntity(const char *entity_name) {
    unsigned id = EntityManager::spawnEntity(entity_name);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

unsigned ObjectManager::spawnObject(const char *object_name) {
    _checkUninitialized(_initialized);

    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();

    // get type information
    ScriptInfo &scriptinfo = _scriptinfos[object_name];
    EntityInfo &entityinfo = _entityinfos[object_name];
    ObjectInfo &objectinfo = _objectinfos[object_name];

    // push to internal storage
    Object *object = objectinfo._allocator();
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(object);
    _objectvalues[id] = ObjectValues{object};
    _entityvalues[id] = EntityValues{object};
    _scriptvalues[id] = ScriptValues{id, object_name, object, scriptinfo._group};
    
    // set up object
    _objectSetup(object, objectinfo, entityinfo, scriptinfo, id);
    
    // call callback if it exists
    if (objectinfo._spawn_callback)
        objectinfo._spawn_callback(object);
    
    return id;
}

void ObjectManager::addObject(std::function<Object*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, const char *animation_name, const char *filter_name, std::function<void(Object*)> spawn_callback) {
    _checkUninitialized(_initialized);
    
    if (!hasObject(name)) {
        addEntity(allocator, name, group, force_enqueue, force_removeonkill, animation_name, nullptr);
        _objectinfos[name] = ObjectInfo{
            filter_name,
            allocator,
            spawn_callback
        };

    } else
        throw std::runtime_error("Attempt to add already added Object name");
}

void ObjectManager::remove(unsigned id) {
    _checkUninitialized(_initialized);
    
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (!_ids.at(id))
        throw InactiveIDException();

    // get info
    ScriptValues &scriptvalues = _scriptvalues[id];
    EntityValues &entityvalues = _entityvalues[id];
    ObjectValues &objectvalues = _objectvalues[id];

    // remove from object-related, entity-related and script-related systems
    _objectRemoval(objectvalues, entityvalues, scriptvalues);
}