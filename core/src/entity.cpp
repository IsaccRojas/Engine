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

void EntityScript::_base() {
    _baseEntity();
}

void EntityScript::_kill() {
    _killEntity();
}

Entity &EntityScript::entity() {
    return *_entity;
};

// --------------------------------------------------------------------------------------------------------------------------

unsigned EntityExecutor::EntityScriptEnqueue::spawn() {
    return _entityexecutor->spawnEntityScript(_name.c_str(), _execution_queue, _tag);
}

EntityExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag) :
    ScriptEnqueue(nullptr, name, execution_queue, tag), _entityexecutor(entityexecutor)
{}

void EntityExecutor::_setupEntityScript(EntityScript *entityscript) {
    // set up entityscript fields
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

void EntityExecutor::addEntityScript(EntityScriptAllocatorInterface *allocator, const char *name, std::function<void(Script*)> spawn_callback, std::function<void(Script*)> remove_callback) {
    if (!hasAdded(name)) {
        Executor::add(nullptr, name, spawn_callback, remove_callback);
        _entityscriptinfos[name] = EntityScriptInfo{allocator};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

unsigned EntityExecutor::spawnEntityScript(const char *entityscript_name, int execution_queue, int tag) {
    // allocate instance and set it up
    EntityScript *entityscript = _entityscriptinfos[entityscript_name]._allocator->_allocate(tag);
    _setupScript(entityscript, entityscript_name, execution_queue, tag);
    _setupEntityScript(entityscript);

    return entityscript->getExecutorID();
}

void EntityExecutor::enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, int tag) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, execution_queue, tag));
}

// --------------------------------------------------------------------------------------------------------------------------

/*

class Entity {
   friend EntityManager;

   EntityManager *_entitymanager;
   std::list<Entity*>::iterator _this_iter;
   std::string _group;
   std::unordered_map<const char*, float> _data_values;
   std::vector<unsigned> _script_ids;
   std::vector<unsigned> _quad_ids;
   std::vector<unsigned> _box_ids;

   std::vector<Quad*> _quads;
   std::vector<Box*> _boxes;
public:
   Entity();
   ~Entity();

   EntityManager &manager();
   std::vector<Quad*> &quads();
   std::vector<Box*> &boxes();
};

*/

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

    // instantiate each EntityScript in info and push to entity's storage
    for (const EntityScriptArgs &a : ei._entityscript_args) {
        entity->_script_ids.push_back(_entityexecutor->spawnEntityScript(a.entityscript_name, a.execution_queue, a.tag));
    }

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

    _entities[ei._group.c_str()].push_back(entity);
    return entity;
}

void EntityManager::removeEntity(Entity *entity) {
    // TODO: properly handle removal with respect to Executor

    _entities[entity->_group.c_str()].erase(entity->_this_iter);
}