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
    return _entityexecutor->spawnEntityScript(_name.c_str(), _execution_queue, _tag, _transform);
}

EntityExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, Transform transform) :
    ScriptEnqueue(nullptr, name, execution_queue, tag), _entityexecutor(entityexecutor), _transform(transform)
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

unsigned EntityExecutor::spawnEntityScript(const char *entityscript_name, int execution_queue, int tag, Transform transform) {
    // allocate instance and set it up
    EntityScript *entityscript = _entityscriptinfos[entityscript_name]._allocator->_allocate(tag);
    _setupScript(entityscript, entityscript_name, execution_queue, tag);
    _setupEntityScript(entityscript);

    return entityscript->getExecutorID();
}

void EntityExecutor::enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, int tag, Transform transform) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, execution_queue, tag, transform));
}

// --------------------------------------------------------------------------------------------------------------------------

/*
Manager::Manager(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box) : _initialized(false) { init(executor, glenv, physspace_box); }
Manager::Manager() : _executor(nullptr), _glenv(nullptr), _physspace_box(nullptr), _initialized(false) {}

void Manager::init(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box) {
    if (_initialized)
        throw InitializedException();
    
    _executor = executor;
    _glenv = glenv;
    _physspace_box = physspace_box;
    _initialized = true;
}

void Manager::uninit() {
    if (!_initialized)
        return;
    
    _executor = nullptr;
    _glenv = nullptr;
    _physspace_box = nullptr;
    _schemes.clear();
    _initialized = false;
}

void Manager::addScheme(Scheme s, const char *name) {
    _schemes[name] = s;
}

void Manager::instScheme(const char *name) {
    Scheme &s = _schemes[name];

    for (const ScriptArgs &a : s._script_args)
        _executor->spawnScript(a.script_name, a.execution_queue, a.tag);
    for (const QuadArgs &a : s._quad_args)
        _glenv->genQuad(a.pos, a.scale, a.color, a.type, a.animation_name, a.texpos, a.texsize, a.innerrad);
    for (const BoxArgs &a : s._box_args)
        _physspace_box->push(a.transf, a.vel, a.callback, a.filter_name);
}
*/