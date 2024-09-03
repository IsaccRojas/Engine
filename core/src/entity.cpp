#include "../include/entity.hpp"

Entity::Entity() : 
    Script(),
    _glenv(nullptr),
    _quad(nullptr),
    _frame(nullptr),
    _quad_id(-1),
    _first_step(true),
    _glenv_ready(false),
    _quad_ready(false),
    _entitymanager(nullptr)
{}
Entity::~Entity() {
    // try erasing existing quad
    removeQuad();
}

void Entity::_init() {
    if (_glenv_ready)
        _initEntity();
}

void Entity::_base() {
    if (_glenv_ready)
        _baseEntity();
}

void Entity::_kill() {
    if (_glenv_ready)
        _killEntity();
}

void Entity::_initEntity() {}
void Entity::_baseEntity() {}
void Entity::_killEntity() {}

AnimationState &Entity::getAnimState() { return _animstate; }

void Entity::stepAnim() {
    if (_glenv_ready) {
        if (!_first_step) {
            // step animation and retrieve current frame
            _animstate.step();
            _frame = _animstate.getCurrent();
        } else {
            _first_step = false;
        }

        // only attempt to write animation data if quad is available
        if (_quad_ready) {
            // write frame data to quad
            _quad->texpos.v = _frame->texpos;
            _quad->texsize.v = _frame->texsize;
        }
    }
}

void Entity::entitySetup(GLEnv *glenv, Animation *animation) {
    // try erasing existing quad
    removeQuad();

    _glenv = glenv;
    _glenv_ready = true;

    _animstate.setAnimation(animation);
    _frame = _animstate.getCurrent();

    // generate new quad
    genQuad(glm::vec3(0.0f), glm::vec3(0.0f));
}

void Entity::genQuad(glm::vec3 pos, glm::vec3 scale) {
    if (_glenv_ready) {
        // erase existing quad
        if (_quad_ready)
            removeQuad();

        // get quad data
        _quad_id = _glenv->genQuad(pos, scale, _frame->texpos, _frame->texsize);
        
        // if successful, retrieve quad
        if (_quad_id >= 0) {
            _quad = _glenv->get(_quad_id);
            _quad_ready = true;
        }
    }

}

int Entity::removeQuad() {
    if (_quad_ready) {
        if (_glenv->remove(_quad_id)) {
            std::cerr << "WARN: attempt to remove Quad ID " << _quad_id << " from GLEnv " << &_glenv << " within Entity " << this << " failed" << std::endl;
            return -1;
        }

        _quad_id = -1;
        _quad_ready = false;
    }
    // no problem
    return 0;
}

Quad *Entity::getQuad() { 
    if (_quad_ready)
        return _quad;
    else
        return nullptr;
}

EntityManager *Entity::getManager() { return _entitymanager; }

// --------------------------------------------------------------------------------------------------------------------------

EntityManager::EntityManager(int maxcount) : ScriptManager(maxcount), _glenv(nullptr), _animations(nullptr) {
    for (int i = 0; i < maxcount; i++)
        _entityvalues.push_back(EntityValues{nullptr});
}
EntityManager::~EntityManager() {}

void EntityManager::_entitySetup(Entity *entity, EntityInfo &entityinfo, ScriptInfo &scriptinfo, int id) {
    _scriptSetup(entity, scriptinfo, id);

    if (_glenv && _animations)
        entity->entitySetup(_glenv, &(*_animations)[entityinfo._animation_name]);
            
    entity->_entitymanager = this;
}
void EntityManager::_entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues) {
    _scriptRemoval(scriptvalues);
    if (entityvalues._entity_ref)
        entityvalues._entity_ref->removeQuad();
}

bool EntityManager::hasEntity(const char *entityname) { return !(_entityinfos.find(entityname) == _entityinfos.end()); }

Entity *EntityManager::getEntity(int id) {
    if (id >= 0 && _ids.at(id))
        return _entityvalues[id]._entity_ref;
    else
        return nullptr;
}

int EntityManager::spawnScript(const char *scriptname) {
    int id = ScriptManager::spawnScript(scriptname);
    _entityvalues[id] = EntityValues{nullptr};
    return id;
}

int EntityManager::spawnEntity(const char *entityname) {
    // fail if exceeding max size
    if (_ids.fillSize() >= _maxcount) {
        std::cerr << "WARN: limit reached in EntityManager " << this << std::endl;
        return -1;
    }

    // get entity information
    ScriptInfo &scriptinfo = _scriptinfos[entityname];
    EntityInfo &entityinfo = _entityinfos[entityname];

    // push to internal storage
    Entity *entity = entityinfo._allocator();
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _entityvalues[id] = EntityValues{entity};
    _scriptvalues[id] = ScriptValues{id, entityname, entity};
    
    // set up entity
    _entitySetup(entity, entityinfo, scriptinfo, id);
    
    // call callback if it exists
    if (entityinfo._spawncallback)
        entityinfo._spawncallback(entity);
    
    return id;
}

void EntityManager::addEntity(std::function<Entity*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, const char *animation_name, std::function<void(Entity*)> spawn_callback) {
    if (!hasEntity(name) && !hasScript(name)) {
        _entityinfos[name] = EntityInfo{
            animation_name,
            allocator,
            spawn_callback
        };
        addScript(allocator, name, group, force_enqueue, force_removeonkill, nullptr);
    }
}

void EntityManager::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Manager " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Manager " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        // get info
        ScriptValues &scriptvalues = _scriptvalues[id];
        EntityValues &entityvalues = _entityvalues[id];

        // remove from entity-related and script-related systems
        _entityRemoval(entityvalues, scriptvalues);
        _entityvalues[id] = EntityValues{nullptr};
    }
}

void EntityManager::setGLEnv(GLEnv *glenv) { if (!_glenv) _glenv = glenv; }
void EntityManager::setAnimations(std::unordered_map<std::string, Animation> *animations) { if (!_animations) _animations = animations; }