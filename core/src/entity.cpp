#include "../include/entity.hpp"

Entity::Entity(Entity &&other) {
    operator=(std::move(other));
}
Entity::Entity() : 
    Script(),
    _entityexecutor(nullptr),
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

EntityExecutor *Entity::getExecutor() { return _entityexecutor; }

// --------------------------------------------------------------------------------------------------------------------------

int EntityExecutor::EntityEnqueue::spawn() {
    return _entityexecutor->_spawnEntity(_name.c_str(), _execution_queue, _tag, _pos);
}

EntityExecutor::EntityEnqueue::EntityEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, glm::vec3 pos) :
    ScriptEnqueue(nullptr, name, execution_queue, tag), _entityexecutor(entityexecutor), _pos(pos)
{}

void EntityExecutor::_setupEntity(unsigned id, Entity *entity, const char *entity_name) {
    // get information
    EntityInfo &info = _entityinfos[entity_name];

    // set up entity fields
    entity->_entityexecutor = this;
    entity->_glenv = _glenv;
    entity->_quad_id = _glenv->genQuad(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f));
    entity->_quad = _glenv->getQuad(entity->_quad_id);
    entity->_animationstate.setAnimation(&(*_animations)[info._animation_name]);
    entity->_frame = entity->_animationstate.getCurrent();

    _entityvalues[id] = EntityValues{entity};
}

unsigned EntityExecutor::_spawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    // allocate instance and set it up
    Entity *entity = _entityinfos[entity_name]._allocator->_allocate(tag);
    unsigned id = _setupScript(entity, entity_name, execution_queue);
    _setupEntity(id, entity, entity_name);
    entity->getQuad()->bv_pos.v = pos;

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
        _entityvalues = other._entityvalues;
        _glenv = other._glenv;
        _animations = other._animations;

        // safe as there are no deallocations (unique_ptrs should be moved by this point)
        other.uninit();
    }
    return *this;
}

void EntityExecutor::init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations) {
    Executor::init(max_count, queues);

    _entityvalues = std::vector<EntityValues>(max_count, EntityValues{nullptr});
    _glenv = glenv;
    _animations = animations;
}

void EntityExecutor::uninit() {
    Executor::uninit();

    _entityinfos.clear();
    _entityvalues.clear();
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

void EntityExecutor::enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos) {
    _pushSpawnEnqueue(new EntityEnqueue(this, entity_name, execution_queue, tag, pos));
}

Entity *EntityExecutor::getEntity(unsigned id) {
    _checkID(id);
    return _entityvalues[id]._entity_ref;
}