#include "../include/object.hpp"

Object::Object(Object &&other) {
    operator=(std::move(other));
}
Object::Object() : 
    Entity(),
    _physenv(nullptr),
    _box(nullptr),
    _box_id(-1)
{}
Object::~Object() {
    // try removing existing Quad
    _objectRemove();
}

Object &Object::operator=(Object &&other) {
    if (this != &other) {
        Entity::operator=(std::move(other));
        _physenv = other._physenv;
        _box = other._box;
        _box_id = other._box_id;
        other._physenv = nullptr;
        other._box = nullptr;
        other._box_id = -1;
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

void Object::_objectRemove() {
    // remove Object from owning PhysEnv (if this has a PhysEnv reference, it must have a valid Box so don't need to check)
    if (_physenv)
        _physenv->remove(_box_id);
    
    _physenv = nullptr;
    _box = nullptr;
    _box_id = -1;
}

Box *Object::getBox() { return _box; }

// --------------------------------------------------------------------------------------------------------------------------

void ObjectExecutor::_setupObject(Object *object, Filter *filter) {
    object->_physenv = _physenv;
    object->_box_id = _physenv->genBox(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), std::bind(Object::_collisionObject, object, std::placeholders::_1));
    object->_box = _physenv->getBox(object->_box_id);
}

unsigned ObjectExecutor::_spawnObject(const char *object_name, int execution_queue, int tag, glm::vec3 pos) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();

    // get type information
    ScriptInfo &scriptinfo = _scriptinfos[object_name];
    EntityInfo &entityinfo = _entityinfos[object_name];
    ObjectInfo &objectinfo = _objectinfos[object_name];

    // push to storage
    Object *object = objectinfo._allocator->_allocate(tag);
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(object);
    _scriptvalues[id] = ScriptValues{object_name, scriptinfo._group};

    // set up script-level, entity-level, and object-level values
    _setupScript(object, id, scriptinfo._removeonkill, scriptinfo._group);
    _setupEntity(object, &(*_animations)[entityinfo._animation_name]);
    _setupObject(object, &(*_filters)[objectinfo._filter_name]);

    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(id, execution_queue);
    
    _count++;
    
    // try spawn callback if it exists
    if (scriptinfo._spawn_callback)
        scriptinfo._spawn_callback(id);

    return id;
}

ObjectExecutor::ObjectExecutor(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters) : EntityExecutor() {
    init(max_count, queues, glenv, animations, physenv, filters);
}
ObjectExecutor::ObjectExecutor(ObjectExecutor &&other) { operator=(std::move(other)); }
ObjectExecutor::ObjectExecutor() : EntityExecutor(), _physenv(nullptr), _filters(nullptr) {}
ObjectExecutor::~ObjectExecutor() { /* automatic destruction is fine */ }

ObjectExecutor &ObjectExecutor::operator=(ObjectExecutor &&other) {
    if (this == &other) {
        EntityExecutor::operator=(std::move(other));
        _objectinfos = other._objectinfos;
        _objectenqueues = other._objectenqueues;
        _physenv = other._physenv;
        _filters = other._filters;

        // safe as there are no deallocations (unique_ptrs should be moved by this point)
        other.uninit();
    }
    return *this;
}

void ObjectExecutor::init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters) {
    EntityExecutor::init(max_count, queues, glenv, animations);
    _physenv = physenv;
    _filters = filters;
}

void ObjectExecutor::uninit() {
    EntityExecutor::uninit();
    if (!_initialized)
        return;

    std::queue<ObjectEnqueue> empty;
    _objectinfos.clear();
    std::swap(_objectenqueues, empty);
    _physenv = nullptr;
    _filters = nullptr;
}

void ObjectExecutor::addObject(ObjectAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::string animation_name, std::string filter_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback) {
    if (!hasAdded(name)) {
        addEntity(nullptr, name, group, removeonkill, animation_name, spawn_callback, remove_callback);
        _objectinfos[name] = ObjectInfo{allocator, filter_name};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

void ObjectExecutor::enqueueSpawn(const char *script_name, int execution_queue, int tag) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{script_name, execution_queue, tag});
    _entityenqueues.push(EntityEnqueue{false, glm::vec3(0.0f)});
    _objectenqueues.push(ObjectEnqueue{false, glm::vec3(0.0f)});
}

void ObjectExecutor::enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{entity_name, execution_queue, tag});
    _entityenqueues.push(EntityEnqueue{true, pos});
    _objectenqueues.push(ObjectEnqueue{false, glm::vec3(0.0f)});
}

void ObjectExecutor::enqueueSpawnObject(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{entity_name, execution_queue, tag});
    _entityenqueues.push(EntityEnqueue{true, pos});
    _objectenqueues.push(ObjectEnqueue{true, pos});
}

std::vector<unsigned> ObjectExecutor::runSpawnQueue() {
    std::vector<unsigned> ids;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();
        EntityEnqueue &entityenqueue = _entityenqueues.front();
        ObjectEnqueue &objectenqueue = _objectenqueues.front();

        if (objectenqueue._is_object)
            ids.push_back(_spawnObject(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag, objectenqueue._pos));
        if (entityenqueue._is_entity)
            ids.push_back(_spawnEntity(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag, entityenqueue._pos));
        else
            ids.push_back(_spawnScript(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag));
        
        _scriptenqueues.pop();
        _entityenqueues.pop();
        _objectenqueues.pop();
    }

    return ids;
}