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

void Entity::_initEntity() {}
void Entity::_baseEntity() {}
void Entity::_killEntity() {}

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

EntityManager *Entity::getManager() { return _entitymanager; }

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
    _entityinfos.clear();
    _entityvalues.clear();
    _glenv = nullptr;
    _animations = nullptr;
}

void EntityManager::_entitySetup(Entity *entity, EntityInfo &entityinfo, ScriptInfo &scriptinfo, unsigned id) {
    _scriptSetup(entity, scriptinfo, id);

    entity->entitySetup(_glenv, &(*_animations)[entityinfo._animation_name]);
    entity->genQuad(glm::vec3(0.0f), glm::vec3(0.0f));
            
    entity->_entitymanager = this;
}
void EntityManager::_entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues) {    
    _scriptRemoval(scriptvalues);
    
    // accessing ref after script-level removal is fine because deletion doesn't actually take place
    entityvalues._entity_ref->removeQuad();

    entityvalues = EntityValues{nullptr};
}

void EntityManager::init(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor) {
    ScriptManager::init(max_count, executor);
    _entityManagerInit(max_count, glenv, animations);
}

void EntityManager::uninit() {
    ScriptManager::uninit();
    _entityManagerUninit();
}

unsigned EntityManager::spawnScript(const char *script_name) {
    unsigned id = ScriptManager::spawnScript(script_name);
    _entityvalues[id] = EntityValues{nullptr};
    return id;
}

unsigned EntityManager::spawnEntity(const char *entity_name) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();
    
    // get entity information
    ScriptInfo &scriptinfo = _scriptinfos[entity_name];
    EntityInfo &entityinfo = _entityinfos[entity_name];

    // push to internal storage
    Entity *entity = entityinfo._allocator();
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _entityvalues[id] = EntityValues{entity};
    _scriptvalues[id] = ScriptValues{id, entity_name, entity};
    
    // set up entity
    _entitySetup(entity, entityinfo, scriptinfo, id);
    
    // call callback if it exists
    if (entityinfo._spawn_callback)
        entityinfo._spawn_callback(entity);
    
    return id;
}

void EntityManager::remove(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (!_ids.at(id))
        throw InactiveIDException();

    // get info
    ScriptValues &scriptvalues = _scriptvalues[id];
    EntityValues &entityvalues = _entityvalues[id];

    // remove from entity-related and script-related systems
    _entityRemoval(entityvalues, scriptvalues);    
}


void EntityManager::addEntity(std::function<Entity*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, const char *animation_name, std::function<void(Entity*)> spawn_callback) {
    if (!hasAddedEntity(name)) {
        addScript(allocator, name, group, force_enqueue, force_removeonkill, nullptr);
        _entityinfos[name] = EntityInfo{
            animation_name,
            allocator,
            spawn_callback
        };

    } else
        throw std::runtime_error("Attempt to add already added Entity name");
}

bool EntityManager::hasAddedEntity(const char *entity_name) { return !(_entityinfos.find(entity_name) == _entityinfos.end()); }

Entity *EntityManager::getEntity(unsigned id) {
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");
    
    if (_ids.at(id))
        return _entityvalues[id]._entity_ref;

    throw InactiveIDException();
}