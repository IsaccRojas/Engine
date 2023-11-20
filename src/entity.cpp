#include "entity.hpp"

Entity::Entity() : _visualpos(glm::vec3(0.0f)), _glenv_ready(false), _first_step(true), _quad_id(-1), _glenv(nullptr), _quad(nullptr), _frame(nullptr), Script() {}
Entity::~Entity() {
    // try erasing existing quad
    eraseQuad();
}

void Entity::_init() {
    _initEntity();
}

void Entity::_base() {
    _baseEntity();

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
            // write frame data to quad and update quad
            _quad->pos.v = _visualpos;
            _quad->texpos.v = _frame->texpos;
            _quad->texsize.v = _frame->texsize;
            _quad->update();
        }

    }
}

void Entity::_kill() {
    _killEntity();
}

void Entity::_initEntity() {}
void Entity::_baseEntity() {}
void Entity::_killEntity() {}

glm::vec3 Entity::getVisPos() { return _visualpos; }
void Entity::setVisPos(glm::vec3 newpos) { _visualpos = newpos; }
AnimationState &Entity::getAnimState() { return _animstate; }

void Entity::entitySetup(GLEnv *glenv, Animation *animation) {
    // try erasing existing quad
    eraseQuad();

    _glenv = glenv;

    _animstate.setAnimation(animation);
    _frame = _animstate.getCurrent();

    _glenv_ready = true;
}

void Entity::genQuad(glm::vec3 pos, glm::vec3 scale) {
    if (_glenv_ready) {
        // erase existing quad
        if (_quad_ready)
            eraseQuad();

        // get quad data
        _quad_id = _glenv->genQuad(pos, scale, _frame->texpos, _frame->texsize);
        
        // if successful, retrieve quad
        if (_quad_id >= 0) {
            _quad = _glenv->get(_quad_id);
            _quad_ready = true;
        }
    }
}

void Entity::eraseQuad() {
    if (_quad_id >= 0) {
        _glenv->erase(_quad_id);
        _quad_id = -1;

        _quad_ready = false;
    }
}

Quad *Entity::getQuad() { 
    if (_quad_ready)
        return _quad;
    else
        return NULL;
}

EntityManager *Entity::getManager() { return _entitymanager; }

// --------------------------------------------------------------------------------------------------------------------------

EntityManager::EntityManager(int maxcount) : ScriptManager(maxcount), _glenv(nullptr), _animations(nullptr) {
    for (int i = 0; i < maxcount; i++)
        _entityvalues.push_back(EntityValues{nullptr});
}
EntityManager::~EntityManager() {}

void EntityManager::_entitySetup(Entity *entity, EntityType &entitytype, ScriptType &scripttype, int id) {
    _scriptSetup(entity, scripttype, id);

    if (entitytype._force_entitysetup)
        if (_glenv && _animations)
            entity->entitySetup(_glenv, &(*_animations)[entitytype._animation_name]);
            
    entity->_entitymanager = this;
}
void EntityManager::_entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues) {
    _scriptRemoval(scriptvalues);
    if (entityvalues._entity_ref)
        entityvalues._entity_ref->eraseQuad();
}

bool EntityManager::hasEntity(const char *entityname) { return !(_entitytypes.find(entityname) == _entitytypes.end()); }

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
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in EntityManager " << this << std::endl;
        return -1;
    }

    // get type information
    ScriptType &scripttype = _scripttypes[entityname];
    EntityType &entitytype = _entitytypes[entityname];

    // push to internal storage
    Entity *entity = entitytype._allocator();
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _entityvalues[id] = EntityValues{entity};
    _scriptvalues[id] = ScriptValues{id, entityname, entity};
    
    // set up entity
    _entitySetup(entity, entitytype, scripttype, id);
    
    return id;
}

void EntityManager::addEntity(std::function<Entity*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name) {
    if (!hasEntity(name) && !hasScript(name)) {
        _entitytypes[name] = EntityType{
            force_entitysetup,
            animation_name,
            allocator
        };
        addScript(allocator, name, type, force_scriptsetup, force_enqueue, force_removeonkill);
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
    
        _ids.erase_at(id);
    }
}

void EntityManager::setGLEnv(GLEnv *glenv) { if (!_glenv) _glenv = glenv; }
void EntityManager::setAnimations(std::unordered_map<std::string, Animation> *animations) { if (!_animations) _animations = animations; }