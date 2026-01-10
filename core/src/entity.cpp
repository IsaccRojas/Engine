#include "../include/entity.hpp"

EntityScript::EntityScript(EntityScript &&other) {
    operator=(std::move(other));
    _entity = other._entity;
    other._entity = nullptr;
}
EntityScript::EntityScript() : 
    Script(), _entity(nullptr)
{}
EntityScript::~EntityScript() { /* automatic destruction is fine */ }

EntityScript &EntityScript::operator=(EntityScript &&other) {
    if (this != &other) {
        Script::operator=(std::move(other));
        _entity = other._entity;
        other._entity = nullptr;
    }
    return *this;
}

void EntityScript::_init() {
    _initEntity();
}

void EntityScript::_exec() {
    _execEntity();
}

void EntityScript::_kill() {
    _killEntity();
    if (_entity)
        _entity->_script_killed = true;
}

void EntityScript::_update() {
    _updateEntity();
}

Entity &EntityScript::entity() {
    return *_entity;
};

bool EntityScript::hasEntity() {
    return !(_entity == nullptr);
}

void EntityScript::receive(Entity *entity, std::string message) {
    _receive(entity, message);
}

// --------------------------------------------------------------------------------------------------------------------------

EntityScriptView::EntityScriptView(EntityScript *entityscript) : ScriptView(entityscript), _entityscript(entityscript) {}
void EntityScriptView::receive(Entity *entity, std::string message) {
    _entityscript->receive(entity, message);
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptView EntityExecutor::EntityScriptEnqueue::spawn() {
    return _entityexecutor->spawnEntityScript(_name.c_str(), _execution_queue, _entity);
}

EntityExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, Entity *entity) :
    ScriptEnqueue(nullptr, name, execution_queue), _entityexecutor(entityexecutor), _entity(entity)
{}

void EntityExecutor::_setupEntityScript(EntityScript *entityscript, Entity *entity) {
    entityscript->_entity = entity;
}

EntityExecutor::EntityExecutor(unsigned queues) : Executor() { 
    init(queues);
}
EntityExecutor::EntityExecutor(EntityExecutor &&other) : Executor() { operator=(std::move(other)); }
EntityExecutor::EntityExecutor() : Executor() {}
EntityExecutor::~EntityExecutor() { /* automatic destruction is fine */ }

EntityExecutor &EntityExecutor::operator=(EntityExecutor &&other) {
    if (this == &other) {
        Executor::operator=(std::move(other));
        _entityscriptinfos = other._entityscriptinfos;
        other._entityscriptinfos.clear();
    }
    return *this;
}

void EntityExecutor::init(unsigned queues) {
    Executor::init(queues);
}

void EntityExecutor::uninit() {
    Executor::uninit();
    _entityscriptinfos.clear();
}

void EntityExecutor::addEntityScript(EntityScriptAllocatorInterface *allocator, const char *name, std::function<void(ScriptView)> spawn_callback, std::function<void(ScriptView)> remove_callback) {
    if (!hasAdded(name)) {
        Executor::add(nullptr, name, spawn_callback, remove_callback);
        _entityscriptinfos[name] = EntityScriptInfo{allocator};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

EntityScriptView EntityExecutor::spawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity) {
    // allocate instance and set it up
    EntityScript *entityscript = _entityscriptinfos[entityscript_name]._allocator->_allocate();
    _setupScript(entityscript, entityscript_name, execution_queue);
    _setupEntityScript(entityscript, entity);

    // run initialization method
    entityscript->runInit();

    return EntityScriptView(entityscript);
}

void EntityExecutor::enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, execution_queue, entity));
}

// --------------------------------------------------------------------------------------------------------------------------

Entity::Entity() : _entitymanager(nullptr), _entityscriptview(nullptr), _script_killed(false) {}
Entity::~Entity() {}

EntityManager &Entity::manager() { return *_entitymanager; }
std::vector<Quad*> &Entity::quads() { return _quads; }
std::vector<Box*> &Entity::boxes() { return _boxes; }
std::unordered_map<const char*, float> &Entity::attributes1f() { return _attributes1f; }
std::unordered_map<const char*, glm::vec2> &Entity::attributes2f() { return _attributes2f; }
std::unordered_map<const char*, glm::vec3> &Entity::attributes3f() { return _attributes3f; }
EntityScriptView &Entity::entityscriptview() { return _entityscriptview; }

// --------------------------------------------------------------------------------------------------------------------------

void EntityManager::_removeEntity(Entity *entity) {
    for (const unsigned &id : entity->_quad_ids)
        _glenv->remove(id);
    for (const unsigned &id : entity->_box_ids)
        _physspace_box->erase(id);

    _entities[entity->_group.c_str()].erase(entity->_this_iter);
}

EntityManager::EntityManager(EntityExecutor *entityexecutor, GLEnv *glenv, PhysSpace<Box> *physspace_box) : _initialized(false) { init(entityexecutor, glenv, physspace_box); }
EntityManager::EntityManager(EntityManager &&other) { operator=(std::move(other)); }
EntityManager::EntityManager() : _entityexecutor(nullptr), _glenv(nullptr), _physspace_box(nullptr), _initialized(false) {}
EntityManager::~EntityManager() { /* automatic destruction is fine */ }

EntityManager &EntityManager::operator=(EntityManager &&other) {
    if (this != &other) {
        _entityinfos = other._entityinfos;

        // clear all lists
        std::vector<const char*> keys;
        for (std::unordered_map<const char *, ManagedList<Entity>>::iterator i = _entities.begin(); i != _entities.end(); ++i)
            keys.push_back(i->first);
        for (const char *s : keys)
            _entities.erase(s);

        // move all lists
        for (std::unordered_map<const char *, ManagedList<Entity>>::iterator i = other._entities.begin(); i != other._entities.end(); ++i)
            _entities[i->first] = std::move(i->second);

        _entityexecutor = other._entityexecutor;
        _glenv = other._glenv;
        _physspace_box = other._physspace_box;

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void EntityManager::init(EntityExecutor *entityexecutor, GLEnv *glenv, PhysSpace<Box> *physspace_box) {
    if (_initialized)
        throw InitializedException();
    
    _entityexecutor = entityexecutor;
    _glenv = glenv;
    _physspace_box = physspace_box;
    _initialized = true;
}

void EntityManager::uninit() {
    if (!_initialized)
        return;
    
    _entityinfos.clear();
    _entities.clear();
    _entityexecutor = nullptr;
    _glenv = nullptr;
    _physspace_box = nullptr;
    _initialized = false;
}

void EntityManager::addEntity(EntityInfo info, const char *name) {
    _entityinfos[name] = info;
}

Entity *EntityManager::spawnEntity(const char *name) {
    EntityInfo &ei = _entityinfos[name];
    Entity *entity = new Entity();

    // instantiate each Quad in info and push to entity's storage
    for (const QuadArgs &a : ei._quad_args) {
        entity->_quad_ids.push_back(_glenv->genQuad(a.pos, a.scale, a.color, a.type, a.animation_name, a.texpos, a.texsize, a.innerrad));
        entity->_quads.push_back(_glenv->getQuad(entity->_quad_ids.back()));
    }

    // instantiate each Box in info and push to entity's storage
    for (const BoxArgs &a : ei._box_args) {
        entity->_box_ids.push_back(_physspace_box->push(a.transf, a.vel, a.callback, a.filter_name));
        entity->_boxes.push_back(_physspace_box->get(entity->_box_ids.back()));
    }

    // instantiate each EntityScript in info and push to entity's storage
    entity->_entityscriptview = _entityexecutor->spawnEntityScript(ei._entityscript_args.entityscript_name, ei._entityscript_args.execution_queue, entity);

    entity->_this_iter = _entities[ei._group.c_str()].push_back(entity);
    entity->_entitymanager = this;
    
    return entity;
}

void EntityManager::checkEntities() {
    std::queue<Entity*> remove_queue;

    // check every script status of every entity in each group list
    for (auto &[name, mlist] : _entities)
        for (auto iter = mlist.begin(); iter != mlist.end(); ++iter)
            if ((*iter)->_script_killed)
                remove_queue.push(*iter);

    while (!remove_queue.empty()) {
        _removeEntity(remove_queue.front());
        remove_queue.pop();
    }
}

std::list<Entity*>::iterator EntityManager::groupBegin(const char *group) {
    return _entities[group].begin();
}

std::list<Entity*>::iterator EntityManager::groupEnd(const char *group) {
    return _entities[group].end();
}