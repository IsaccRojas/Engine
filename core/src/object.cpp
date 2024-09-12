#include "../include/object.hpp"

Object::Object(Object &&other) {
    operator=(std::move(other));
}
Object::Object() : 
    Entity(),
    _objectexecutor(nullptr),
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

ObjectExecutor *Object::getExecutor() { return _objectexecutor; }

// --------------------------------------------------------------------------------------------------------------------------

int ObjectExecutor::ObjectEnqueue::spawn() {
    return _objectexecutor->_spawnObject(_name.c_str(), _execution_queue, _tag, _pos);
}

ObjectExecutor::ObjectEnqueue::ObjectEnqueue(ObjectExecutor *objectexecutor, std::string name, int execution_queue, int tag, glm::vec3 pos) :
    EntityEnqueue(nullptr, name, execution_queue, tag, pos), _objectexecutor(objectexecutor)
{}

void ObjectExecutor::_setupObject(unsigned id, Object *object, const char *object_name) {
    // get information
    ObjectInfo &info = _objectinfos[object_name];

    // set up object information
    object->_objectexecutor = this;
    object->_physenv = _physenv;
    object->_box_id = _physenv->genBox(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), std::bind(Object::_collisionObject, object, std::placeholders::_1));
    object->_box = _physenv->getBox(object->_box_id);
    object->_box->setFilter(&(*_filters)[info._filter_name]);

    _objectvalues[id] = ObjectValues{object};
}

unsigned ObjectExecutor::_spawnObject(const char *object_name, int execution_queue, int tag, glm::vec3 pos) {
    // allocate instance and set it up
    Object *object = _objectinfos[object_name]._allocator->_allocate(tag);
    unsigned id = _setupScript(object, object_name, execution_queue);
    _setupEntity(id, object, object_name);
    _setupObject(id, object, object_name);
    object->getQuad()->bv_pos.v = pos;
    object->getBox()->pos = pos;

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
        _objectvalues = other._objectvalues;
        _physenv = other._physenv;
        _filters = other._filters;

        // safe as there are no deallocations (unique_ptrs should be moved by this point)
        other.uninit();
    }
    return *this;
}

void ObjectExecutor::init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters) {
    EntityExecutor::init(max_count, queues, glenv, animations);

    _objectvalues = std::vector<ObjectValues>(max_count, ObjectValues{nullptr});
    _physenv = physenv;
    _filters = filters;
}

void ObjectExecutor::uninit() {
    EntityExecutor::uninit();

    _objectinfos.clear();
    _objectvalues.clear();
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

void ObjectExecutor::enqueueSpawnObject(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    _pushSpawnEnqueue(new ObjectEnqueue(this, entity_name, execution_queue, tag, pos));
}

Object *ObjectExecutor::getObject(unsigned id) {
    _checkID(id);
    return _objectvalues[id]._object_ref;
}