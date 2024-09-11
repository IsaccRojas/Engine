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
    _first_step(true)
{}
Entity::~Entity() {
    // try removing existing Quad
    _entityRemove();
}

Entity &Entity::operator=(Entity &&other) {
    if (this != &other) {
        Script::operator=(std::move(other));
        _glenv = other._glenv;
        _quad = other._quad;
        _frame = other._frame;
        _quad_id = other._quad_id;
        _first_step = other._first_step;
        _animationstate = other._animationstate;
        other._glenv = nullptr;
        other._quad = nullptr;
        other._frame = nullptr;
        other._quad_id = -1;
        other._first_step = true;
        other._animationstate.setAnimation(nullptr);
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

void Entity::stepAnim() {
    // if this has a GLEnv reference, it must have a valid Quad and Animation so don't need to check those
    if (!_glenv)
        throw std::runtime_error("Attempt to step Animation with null GLEnv reference");

    if (!_first_step) {
        // step animation and retrieve current frame
        _animationstate.step();
        _frame = _animationstate.getCurrent();
    } else {
        _first_step = false;
    }

    // write frame data to quad
    _quad->bv_texpos.v = _frame->texpos;
    _quad->bv_texsize.v = _frame->texsize;       
}

void Entity::_entityRemove() {
    // remove Quad from owning GLEnv (if this has a GLEnv reference, it must have a valid Quad and Animation so don't need to check)
    if (_glenv)
        _glenv->remove(_quad_id);
    
    _glenv = nullptr;
    _quad = nullptr;
    _frame = nullptr;
    _quad_id = -1;
    _first_step = true;
    _animationstate.setAnimation(nullptr);
}

AnimationState &Entity::getAnimState() { return _animationstate; }

Quad *Entity::getQuad() { return _quad; }

// --------------------------------------------------------------------------------------------------------------------------

void EntityExecutor::_setupEntity(Entity *entity, Animation *animation) {
    entity->_glenv = _glenv;
    entity->_quad_id = _glenv->genQuad(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f));
    entity->_quad = _glenv->getQuad(entity->_quad_id);
    entity->_animationstate.setAnimation(animation);
    entity->_frame = entity->_animationstate.getCurrent();
}

unsigned EntityExecutor::_spawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();

    // get type information
    ScriptInfo &scriptinfo = _scriptinfos[entity_name];
    EntityInfo &entityinfo = _entityinfos[entity_name];

    // push to storage
    Entity *entity = entityinfo._allocator->_allocate(tag);
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _scriptvalues[id] = ScriptValues{entity_name, scriptinfo._group};

    // set up script-level and entity-level values
    _setupScript(entity, id, scriptinfo._removeonkill, scriptinfo._group);
    _setupEntity(entity, &(*_animations)[entityinfo._animation_name]);

    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(id, execution_queue);
    
    _count++;
    
    // try spawn callback if it exists
    if (scriptinfo._spawn_callback)
        scriptinfo._spawn_callback(id);

    return id;
}

EntityExecutor::EntityExecutor(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations) : Executor() {
    init(max_count, queues, glenv, animations);
}
EntityExecutor::EntityExecutor(EntityExecutor &&other) { operator=(std::move(other)); }
EntityExecutor::EntityExecutor() : Executor(), _glenv(nullptr), _animations(nullptr) {}
EntityExecutor::~EntityExecutor() { /* automatic destruction is fine */ }

EntityExecutor &EntityExecutor::operator=(EntityExecutor &&other) {
    if (this == &other) {
        Executor::operator=(std::move(other));
        _entityinfos = other._entityinfos;
        _entityenqueues = other._entityenqueues;
        _glenv = other._glenv;
        _animations = other._animations;

        // safe as there are no deallocations (unique_ptrs should be moved by this point)
        other.uninit();
    }
    return *this;
}

void EntityExecutor::init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations) {
    Executor::init(max_count, queues);
    _glenv = glenv;
    _animations = animations;
}

void EntityExecutor::uninit() {
    Executor::uninit();
    if (!_initialized)
        return;

    std::queue<EntityEnqueue> empty;
    _entityinfos.clear();
    std::swap(_entityenqueues, empty);
    _glenv = nullptr;
    _animations = nullptr;
}

void EntityExecutor::addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::string animation_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback) {
    if (!hasAdded(name)) {
        Executor::add(nullptr, name, group, removeonkill, spawn_callback, remove_callback);
        _entityinfos[name] = EntityInfo{allocator, animation_name};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

void EntityExecutor::enqueueSpawn(const char *script_name, int execution_queue, int tag) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{script_name, execution_queue, tag});
    _entityenqueues.push(EntityEnqueue{false, glm::vec3(0.0f)});
}

void EntityExecutor::enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{entity_name, execution_queue, tag});
    _entityenqueues.push(EntityEnqueue{true, pos});
}

std::vector<unsigned> EntityExecutor::runSpawnQueue() {
    std::vector<unsigned> ids;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();
        EntityEnqueue &entityenqueue = _entityenqueues.front();

        if (entityenqueue._is_entity)
            ids.push_back(_spawnEntity(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag, entityenqueue._pos));
        else
            ids.push_back(_spawnScript(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag));
        
        _scriptenqueues.pop();
        _entityenqueues.pop();
    }

    return ids;
}