#include "../include/entity.hpp"

Entity::Entity(Entity &&other) {
    operator=(std::move(other));
}
Entity::Entity() : 
    Script(),
    _glenv(nullptr),
    _quad(nullptr),
    _frame(nullptr),
    _quad_id(-1),
    _first_step(true),
    _entitymanager(nullptr)
{}
Entity::~Entity() {
    // try removing existing Quad
    removeQuad();
}

Entity &Entity::operator=(Entity &&other) {
    if (this != &other) {
        Script::operator=(std::move(other));
        _glenv = other._glenv;
        _quad = other._quad;
        _frame = other._frame;
        _quad_id = other._quad_id;
        _first_step = other._first_step;
        _entitymanager = other._entitymanager;
        other._glenv = nullptr;
        other._quad = nullptr;
        other._frame = nullptr;
        other._quad_id = -1;
        other._first_step = true;
        other._entitymanager = nullptr;
    }
    return *this;
}

void Entity::_init() {
    if (_glenv)
        _initEntity();
}

void Entity::_base() {
    if (_glenv)
        _baseEntity();
}

void Entity::_kill() {
    if (_glenv)
        _killEntity();
}

AnimationState &Entity::getAnimState() { return _animationstate; }

void Entity::stepAnim() {
    if (_glenv) {
        if (!_first_step) {
            // step animation and retrieve current frame
            _animationstate.step();
            _frame = _animationstate.getCurrent();
        } else {
            _first_step = false;
        }

        // only attempt to write animation data if quad is available
        if (_quad) {
            // write frame data to quad
            _quad->bv_texpos.v = _frame->texpos;
            _quad->bv_texsize.v = _frame->texsize;
        }
    } else
        throw std::runtime_error("Attempt to step Animation with null GLEnv reference");
}

void Entity::entitySetup(GLEnv *glenv, Animation *animation) {
    if (!glenv)
        throw std::runtime_error("Attempt to setup Entity with null GLEnv reference");
    if (!animation)
        throw std::runtime_error("Attempt to setup Entity with null Animation reference");

    // try removing existing GLEnv and Animation information
    entityClear();

    _glenv = glenv;
    _animationstate.setAnimation(animation);

    _frame = _animationstate.getCurrent();
}

void Entity::entityClear() {
    // try removing existing Quad
    removeQuad();
    _glenv = nullptr;
    _animationstate.setAnimation(nullptr);
}

void Entity::genQuad(glm::vec3 pos, glm::vec3 scale) {
    if (_glenv) {
        // erase existing Quad
        if (_quad)
            removeQuad();

        // get Quad data
        _quad_id = _glenv->genQuad(pos, scale, _frame->texpos, _frame->texsize);
        _quad = _glenv->getQuad(_quad_id);
    } else
        throw std::runtime_error("Attempt to generate Quad with null GLEnv reference");
}

void Entity::removeQuad() {
    if (_glenv && _quad) {
        _glenv->remove(_quad_id);

        _quad = nullptr;
        _quad_id = -1;
    }
}

Quad *Entity::getQuad() { 
    return _quad;
}

// --------------------------------------------------------------------------------------------------------------------------

//TODO: only signature is complete at the moment
EntityManager::EntityManager(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor) : ScriptManager() {
    init(max_count, glenv, animations, executor);
}
EntityManager::EntityManager(EntityManager &&other) {
    operator=(std::move(other));
}
EntityManager::EntityManager() : ScriptManager(), _glenv(nullptr), _animations(nullptr) {}
EntityManager::~EntityManager() { /* automatic destruction is fine */ }

EntityManager &EntityManager::operator=(EntityManager &&other) {
    if (this != &other) {
        ScriptManager::operator=(std::move(other));
        _entityinfos = other._entityinfos;
        _entityvalues = other._entityvalues;
        _entityenqueues = other._entityenqueues;
        _glenv = other._glenv;
        _animations = other._animations;
        other._entityManagerUninit();
    }
    return *this;
}

void EntityManager::_entityManagerInit(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations) {
    _glenv = glenv;
    _animations = animations;
    for (unsigned i = 0; i < _max_count; i++)
        _entityvalues.push_back(EntityValues{nullptr});
}

void EntityManager::_entityManagerUninit() {
    std::queue<EntityEnqueue> empty;

    _entityinfos.clear();
    _entityvalues.clear();
    std::swap(_entityenqueues, empty);
    _glenv = nullptr;
    _animations = nullptr;
}

void EntityManager::_entitySetup(Entity *entity, EntityInfo &entityinfo, ScriptInfo &scriptinfo, unsigned id, int executor_queue, glm::vec3 entity_pos) {
    _scriptSetup(entity, scriptinfo, id, executor_queue);

    entity->entitySetup(_glenv, &(*_animations)[entityinfo._animation_name]);
    entity->genQuad(entity_pos, glm::vec3(0.0f));
            
    entity->_entitymanager = this;
}

void EntityManager::_entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues) {    
    _scriptRemoval(scriptvalues);
    
    // accessing ref after script-level removal is fine because deletion doesn't actually take place
    entityvalues._entity_ref->removeQuad();

    entityvalues = EntityValues{nullptr};
}

unsigned EntityManager::_spawnScript(const char *script_name, int executor_queue) {
    unsigned id = ScriptManager::_spawnScript(script_name, executor_queue);
    _entityvalues[id] = EntityValues{nullptr};
    return id;
}

unsigned EntityManager::_spawnEntity(const char *entity_name, int executor_queue, glm::vec3 entity_pos) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();
    
    // get entity information
    ScriptInfo &scriptinfo = _scriptinfos[entity_name];
    EntityInfo &entityinfo = _entityinfos[entity_name];

    // push to internal storage
    Entity *entity = entityinfo._allocator->_allocate();
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _entityvalues[id] = EntityValues{entity};
    _scriptvalues[id] = ScriptValues{id, entity_name, entity};
    
    // set up entity
    _entitySetup(entity, entityinfo, scriptinfo, id, executor_queue, entity_pos);
    
    // call callback if it exists
    if (scriptinfo._spawn_callback)
        scriptinfo._spawn_callback(id);
    
    return id;
}

void EntityManager::init(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor) {
    ScriptManager::init(max_count, executor);
    _entityManagerInit(max_count, glenv, animations);
}

void EntityManager::uninit() {
    ScriptManager::uninit();
    _entityManagerUninit();
}

void EntityManager::spawnScriptEnqueue(const char *script_name, int executor_queue, CaptorInterface *captor) {
    ScriptManager::spawnScriptEnqueue(script_name, executor_queue, captor);
    _entityenqueues.push(EntityEnqueue{false, glm::vec3(0.0f)});
}

void EntityManager::spawnEntityEnqueue(const char *script_name, int executor_queue, glm::vec3 entity_pos, CaptorInterface *captor) {
    ScriptManager::spawnScriptEnqueue(script_name, executor_queue, captor);
    _entityenqueues.push(EntityEnqueue{true, entity_pos});
}

std::vector<unsigned> EntityManager::runSpawnQueue() {
    std::vector<unsigned> ids;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();
        EntityEnqueue &entityenqueue = _entityenqueues.front();
        
        if (entityenqueue._valid)
            ids.push_back(_spawnEntity(scriptenqueue._name.c_str(), scriptenqueue._executor_queue, entityenqueue._entity_pos));
        else if (scriptenqueue._valid)
            ids.push_back(_spawnScript(scriptenqueue._name.c_str(), scriptenqueue._executor_queue));

        if (scriptenqueue._captor)
            scriptenqueue._captor->_capture();

        _scriptenqueues.pop();
        _entityenqueues.pop();
    }

    return ids;
}

void EntityManager::remove(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (!_ids.at(id))
        throw InactiveIDException();

    // get values
    ScriptValues &scriptvalues = _scriptvalues[id];
    EntityValues &entityvalues = _entityvalues[id];

    // get info for removal callback
    ScriptInfo &scriptinfo = _scriptinfos[scriptvalues._manager_name];
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(id);

    // check refs
    if (entityvalues._entity_ref)
        _entityRemoval(entityvalues, scriptvalues);     
    else
        _scriptRemoval(scriptvalues);
}


void EntityManager::addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, const char *animation_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)> remove_callback) {
    if (!hasAddedEntity(name)) {
        addScript(allocator, name, group, force_removeonkill, spawn_callback, remove_callback);
        _entityinfos[name] = EntityInfo{
            animation_name,
            allocator
        };

    } else
        throw std::runtime_error("Attempt to add already added Entity name");
}

bool EntityManager::hasAddedEntity(const char *entity_name) { return !(_entityinfos.find(entity_name) == _entityinfos.end()); }

Entity *EntityManager::getEntity(unsigned id) {
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");
    
    if (_ids.at(id)) {
        if (_entityvalues[id]._entity_ref)
            return _entityvalues[id]._entity_ref;
        else
            throw std::runtime_error("Attempt to get non-Entity reference with getEntity()");
    }

    throw InactiveIDException();
}