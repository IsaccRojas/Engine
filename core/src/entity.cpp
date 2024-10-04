#include "../include/entity.hpp"

Entity::Entity(Entity &&other) {
    operator=(std::move(other));
}
Entity::Entity() : 
    Script(),
    _entityexecutor(nullptr)
{}
Entity::~Entity() { /* automatic destruction is fine */ }

Entity &Entity::operator=(Entity &&other) {
    if (this != &other) {
        Script::operator=(std::move(other));
        _entityexecutor = other._entityexecutor;
        other._entityexecutor = nullptr;
    }
    return *this;
}

void Entity::_init() {
    if (_entityexecutor)
        _initEntity();
}

void Entity::_base() {
    if (_entityexecutor)
        _baseEntity();
}

void Entity::_kill() {
    if (_entityexecutor)
        _killEntity();
}

EntityExecutor &Entity::executor() { return *_entityexecutor; }

// --------------------------------------------------------------------------------------------------------------------------

Entity *EntityExecutor::EntityEnqueue::spawn() {
    return _entityexecutor->_spawnEntity(_name.c_str(), _execution_queue, _tag, _transform);
}

EntityExecutor::EntityEnqueue::EntityEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, Transform transform) :
    ScriptEnqueue(nullptr, name, execution_queue, tag), _entityexecutor(entityexecutor), _transform(transform)
{}

void EntityExecutor::_setupEntity(Entity *entity, const char *entity_name) {
    // set up entity fields
    entity->_entityexecutor = this;
}

Entity *EntityExecutor::_spawnEntity(const char *entity_name, int execution_queue, int tag, Transform transform) {
    // allocate instance and set it up
    Entity *entity = _entityinfos[entity_name]._allocator->_allocate(tag);
    _setupScript(entity, entity_name, execution_queue, tag);
    _setupEntity(entity, entity_name);
    entity->transform = transform;

    return entity;
}

EntityExecutor::EntityExecutor(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysSpace<Box> *box_space, PhysSpace<Sphere> *sphere_space, unordered_map_string_Filter_t *filters) : Executor() { 
    init(queues, glenv, animations, box_space, sphere_space, filters);
}
EntityExecutor::EntityExecutor(EntityExecutor &&other) : Executor() { operator=(std::move(other)); }
EntityExecutor::EntityExecutor() : Executor(), _glenv(nullptr), _animations(nullptr) {}
EntityExecutor::~EntityExecutor() { /* automatic destruction is fine */ }

EntityExecutor &EntityExecutor::operator=(EntityExecutor &&other) {
    if (this == &other) {
        Executor::operator=(std::move(other));
        _entityinfos = other._entityinfos;
        _glenv = other._glenv;
        _animations = other._animations;
        _box_space = other._box_space;
        _sphere_space = other._sphere_space;
        _filters = other._filters;
        other._entityinfos.clear();
        other._glenv = nullptr;
        other._animations = nullptr;
        other._box_space = nullptr;
        other._sphere_space = nullptr;
        other._filters = nullptr;
    }
    return *this;
}

void EntityExecutor::init(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysSpace<Box> *box_space, PhysSpace<Sphere> *sphere_space, unordered_map_string_Filter_t *filters) {
    Executor::init(queues);
    _glenv = glenv;
    _animations = animations;
    _box_space = box_space;
    _sphere_space = sphere_space;
    _filters = filters;
}

void EntityExecutor::uninit() {
    Executor::uninit();
    _entityinfos.clear();
    _glenv = nullptr;
    _animations = nullptr;
    _box_space = nullptr;
    _sphere_space = nullptr;
    _filters = nullptr;
}

void EntityExecutor::addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::function<void(Script*)> spawn_callback, std::function<void(Script*)>  remove_callback) {
    if (!hasAdded(name)) {
        Executor::add(nullptr, name, group, removeonkill, spawn_callback, remove_callback);
        _entityinfos[name] = EntityInfo{allocator};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

void EntityExecutor::enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, Transform transform) {
    _pushSpawnEnqueue(new EntityEnqueue(this, entity_name, execution_queue, tag, transform));
}

GLEnv &EntityExecutor::glenv() { return *_glenv; }
unordered_map_string_Animation_t &EntityExecutor::animations() { return *_animations; }
PhysSpace<Box> &EntityExecutor::boxspace() { return *_box_space; }
PhysSpace<Sphere> &EntityExecutor::spherespace() { return *_sphere_space; }
unordered_map_string_Filter_t &EntityExecutor::filters() { return *_filters; }