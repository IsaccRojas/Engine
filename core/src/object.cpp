#include "../include/object.hpp"

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
        _box = _physenv->getBox(_box_id);
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
        _objectenqueues = other._objectenqueues;
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
    std::queue<ObjectEnqueue> empty;

    _objectinfos.clear();
    _objectvalues.clear();
    std::swap(_objectenqueues, empty);
    _physenv = nullptr;
    _filters = nullptr;
}

void ObjectManager::_objectSetup(Object *object, ObjectInfo &objectinfo, EntityInfo &entityinfo, ScriptInfo &scriptinfo, unsigned id, int executor_queue, glm::vec3 object_pos) {
    _entitySetup(object, entityinfo, scriptinfo, id, executor_queue, object_pos);

    object->objectSetup(_physenv, &(*_filters)[objectinfo._filter_name]);
    object->genBox(object_pos, glm::vec3(0.0f), glm::vec3(0.0f));

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

unsigned ObjectManager::spawnScript(const char *script_name, int executor_queue) {
    unsigned id = EntityManager::spawnScript(script_name, executor_queue);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

unsigned ObjectManager::spawnEntity(const char *entity_name, int executor_queue, glm::vec3 entity_pos) {
    unsigned id = EntityManager::spawnEntity(entity_name, executor_queue, entity_pos);
    _objectvalues[id] = ObjectValues{nullptr};
    return id;
}

unsigned ObjectManager::spawnObject(const char *object_name, int executor_queue, glm::vec3 object_pos) {
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
    _objectSetup(object, objectinfo, entityinfo, scriptinfo, id, executor_queue, object_pos);
    
    // call callback if it exists
    if (objectinfo._spawn_callback)
        objectinfo._spawn_callback(object);
    
    return id;
}

void ObjectManager::spawnScriptEnqueue(const char *script_name, int executor_queue) {
    EntityManager::spawnScriptEnqueue(script_name, executor_queue);
    _objectenqueues.push(ObjectEnqueue{false, glm::vec3(0.0f)});
}

void ObjectManager::spawnEntityEnqueue(const char *script_name, int executor_queue, glm::vec3 entity_pos) {
    EntityManager::spawnEntityEnqueue(script_name, executor_queue, entity_pos);
    _objectenqueues.push(ObjectEnqueue{false, glm::vec3(0.0f)});
}

void ObjectManager::spawnObjectEnqueue(const char *script_name, int executor_queue, glm::vec3 object_pos) {
    EntityManager::spawnEntityEnqueue(script_name, executor_queue, object_pos);
    _objectenqueues.push(ObjectEnqueue{true, object_pos});
}

std::vector<unsigned> ObjectManager::runSpawnQueue() {
    std::vector<unsigned> ids;
    
    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();
        EntityEnqueue &entityenqueue = _entityenqueues.front();
        ObjectEnqueue &objectenqueue = _objectenqueues.front();
        
        if (objectenqueue._valid)
            ids.push_back(spawnObject(scriptenqueue._name.c_str(), scriptenqueue._executor_queue, objectenqueue._object_pos));
        else if (entityenqueue._valid)
            ids.push_back(spawnEntity(scriptenqueue._name.c_str(), scriptenqueue._executor_queue, entityenqueue._entity_pos));
        else if (scriptenqueue._valid)
            ids.push_back(spawnScript(scriptenqueue._name.c_str(), scriptenqueue._executor_queue));
        
        _scriptenqueues.pop();
        _entityenqueues.pop();
        _objectenqueues.pop();
    }

    return ids;
}

void ObjectManager::remove(unsigned id) {
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

void ObjectManager::addObject(std::function<Object*(void)> allocator, const char *name, int group, bool force_removeonkill, const char *animation_name, const char *filter_name, std::function<void(Object*)> spawn_callback) {
    if (!hasAddedObject(name)) {
        addEntity(allocator, name, group, force_removeonkill, animation_name, nullptr);
        _objectinfos[name] = ObjectInfo{
            filter_name,
            allocator,
            spawn_callback
        };

    } else
        throw std::runtime_error("Attempt to add already added Object name");
}

bool ObjectManager::hasAddedObject(const char *object_name) { return !(_objectinfos.find(object_name) == _objectinfos.end()); }

Object *ObjectManager::getObject(unsigned id) {
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");
    
    if (_ids.at(id))
        return _objectvalues[id]._object_ref;

    throw InactiveIDException();
}

