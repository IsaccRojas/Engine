#include "../include/entity.hpp"

EntityScriptInterface::EntityScriptInterface(EntityScriptInterface&& other) {
    operator=(std::move(other));
    _entity = other._entity;
    other._entity = nullptr;
}
EntityScriptInterface::EntityScriptInterface() : 
    ScriptInterface(), _entity(nullptr)
{}
EntityScriptInterface::~EntityScriptInterface() { /* automatic destruction is fine */ }

EntityScriptInterface& EntityScriptInterface::operator=(EntityScriptInterface&& other) {
    if (this != &other) {
        ScriptInterface::operator=(std::move(other));
        _entity = other._entity;
        other._entity = nullptr;
    }
    return *this;
}

void EntityScriptInterface::_init() {
    _initEntity();
}

void EntityScriptInterface::_exec() {
    _execEntity();
}

void EntityScriptInterface::_kill() {
    _killEntity();

    // null all inserted nullable references
    for (EntityScriptInterface** r : _nullablerefs)
        *r = nullptr;
}

void EntityScriptInterface::_update() {
    _updateEntity();
}

Entity& EntityScriptInterface::entity() {
    return *_entity;
};

void EntityScriptInterface::receive(Entity* entity, std::string message) {
    _receive(entity, message);
}

void EntityScriptInterface::collide(Entity* entity) {
    _collide(entity);
}

void EntityScriptInterface::attachNullableRef(EntityScriptInterface** ref) {
    _nullablerefs.insert(ref);
}

void EntityScriptInterface::detachNullableRef(EntityScriptInterface** ref) {
    _nullablerefs.erase(ref);
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptInterface* EntityScriptExecutor::EntityScriptEnqueue::spawn() {
    return _entityscriptexecutor->spawnEntityScript(_name.c_str(), _entity);
}

EntityScriptExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityScriptExecutor* entityscriptexecutor, std::string name, Entity* entity) :
    ScriptEnqueue(nullptr, name), _entityscriptexecutor(entityscriptexecutor), _entity(entity)
{}

void EntityScriptExecutor::_setupEntityScript(EntityScriptInterface* entityscript, Entity* entity) {
    entityscript->_entity = entity;
}

EntityScriptExecutor::EntityScriptExecutor(unsigned queues) : ScriptExecutor() { 
    init(queues);
}
EntityScriptExecutor::EntityScriptExecutor(EntityScriptExecutor &&other) : ScriptExecutor() { operator=(std::move(other)); }
EntityScriptExecutor::EntityScriptExecutor() : ScriptExecutor() {}
EntityScriptExecutor::~EntityScriptExecutor() { /* automatic destruction is fine */ }

EntityScriptExecutor& EntityScriptExecutor::operator=(EntityScriptExecutor&& other) {
    if (this == &other) {
        ScriptExecutor::operator=(std::move(other));
        _entityscriptinfos = other._entityscriptinfos;
        other._entityscriptinfos.clear();
    }
    return *this;
}

void EntityScriptExecutor::init(unsigned queues) {
    ScriptExecutor::init(queues);
}

void EntityScriptExecutor::uninit() {
    ScriptExecutor::uninit();
    _entityscriptinfos.clear();
}

void EntityScriptExecutor::addEntityScript(EntityScriptInfo entityscriptinfo, const char* name) {
    if (!hasAdded(name)) {
        ScriptExecutor::addScript(ScriptInfo{
            entityscriptinfo.allocator, 
            entityscriptinfo.preferred_queue, 
            entityscriptinfo.auto_enqueue, 
            entityscriptinfo.spawn_callback, 
            entityscriptinfo.remove_callback
        }, name);
        _entityscriptinfos[name] = entityscriptinfo;
    } else
        throw std::runtime_error("Attempt to add already added name");
}

EntityScriptInterface* EntityScriptExecutor::spawnEntityScript(const char* entityscript_name, Entity* entity) {
    // allocate instance and set it up
    EntityScriptInterface* entityscript = _entityscriptinfos[entityscript_name].allocator->_allocate();
    _setupScript(entityscript, entityscript_name, _entityscriptinfos[entityscript_name].allocator);
    _setupEntityScript(entityscript, entity);

    // run initialization method
    entityscript->runInit();

    return entityscript;
}

void EntityScriptExecutor::enqueueSpawnEntityScript(const char* entityscript_name, Entity* entity) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, entity));
}

// --------------------------------------------------------------------------------------------------------------------------

Entity::Entity() : _entitymanager(nullptr), _kill_started(false) {}
Entity::~Entity() {}

void Entity::kill() {
    if (!_kill_started) {
        _kill_started = true;
        for (auto& es : _entityscripts)
            if (es != nullptr)
                es->enqueueKill();
    }
}

std::vector<EntityScriptInterface*>& Entity::entityscripts() { return _entityscripts; }
std::vector<Quad*>& Entity::quads() { return _quads; }
std::vector<EntityCollider*>& Entity::entitycolliders() { return _entitycolliders; }

Transform& Entity::globaltransform() { return _globaltransform; }

const char* Entity::getName() { return _entity_name.c_str(); }

const char* Entity::getGroup() { return _group.c_str(); }

bool Entity::getKillStarted() { return _kill_started; }

// --------------------------------------------------------------------------------------------------------------------------

EntityScriptAllocatorInterface::EntityScriptAllocatorInterface() {}

// --------------------------------------------------------------------------------------------------------------------------

EntityCollider::EntityCollider(EntityCollider&& other) { operator=(std::move(other)); }
EntityCollider::EntityCollider() :
    _collisionspace(nullptr),
    _collision_enabled(false),
    _transformation_is_reset(true),
    _base_pos(glm::vec3(0.0f)),
    _base_scale(glm::vec3(1.0f)),
    _cur_pos(glm::vec3(0.0f)),
    _cur_scale(glm::vec3(1.0f)),
    _prev_pos(glm::vec3(0.0f)),
    _prev_scale(glm::vec3(1.0f)),
    _entity(nullptr)
{}
EntityCollider::~EntityCollider() {}

EntityCollider& EntityCollider::operator=(EntityCollider&& other) {
    if (this != &other) {
        _collisionspace = other._collisionspace;
        _this_iter = other._this_iter;
        _filterstate = other._filterstate;
        _collision_enabled = other._collision_enabled;
        _transformation_is_reset = other._transformation_is_reset;
        _base_pos = other._base_pos;
        _base_scale = other._base_scale;
        _cur_pos = other._cur_pos;
        _cur_scale = other._cur_scale;
        _prev_pos = other._prev_pos;
        _prev_scale = other._prev_scale;
        _entity = other._entity;
        other._collisionspace = nullptr;
        other._collision_enabled = false;
        other._transformation_is_reset = true;
        other._base_pos = glm::vec3(0.0f);
        other._base_scale = glm::vec3(1.0f);
        other._cur_pos = glm::vec3(0.0f);
        other._cur_scale = glm::vec3(1.0f);
        other._prev_pos = glm::vec3(0.0f);
        other._prev_scale = glm::vec3(1.0f);
        other._entity = nullptr;
    }
    return *this;
}

void EntityCollider::_storePrevTransformation() {
    if (!_transformation_is_reset) {
        _prev_pos = _cur_pos;
        _prev_scale = _cur_scale;
    }
}

void EntityCollider::resetTransformation() {
    _storePrevTransformation();
    _cur_pos = _base_pos;
    _cur_scale = _base_scale;
    _transformation_is_reset = true;
}

void EntityCollider::applyTransform(Transform transform) {
    _storePrevTransformation();
    _cur_pos += transform.pos;
    _cur_scale *= transform.scale;
    _transformation_is_reset = false;
}

Transform EntityCollider::getBaseTransformation() {
    return Transform{_base_pos, _base_scale};
}

Transform EntityCollider::getCurrentTransformation() {
    return Transform{_cur_pos, _cur_scale};
}

Transform EntityCollider::getPrevTransformation() {
    return Transform{_prev_pos, _prev_scale};
}

void EntityCollider::attachNullableEntityScript(EntityScriptInterface** script) {
    _scripts.insert(script);
}

void EntityCollider::detachNullableEntityScript(EntityScriptInterface** script) {
    _scripts.erase(script);
}

FilterState& EntityCollider::filterstate() { return _filterstate; }
Entity& EntityCollider::entity() {return *_entity; }
bool& EntityCollider::collision_enabled() { return _collision_enabled; }

// --------------------------------------------------------------------------------------------------------------------------

CollisionSpace::CollisionSpace() : _initialized(false) {}
CollisionSpace::CollisionSpace(CollisionSpace&& other) { operator=(std::move(other)); }
CollisionSpace::~CollisionSpace() { /* automatic destruction is fine */ }

CollisionSpace& CollisionSpace::operator=(CollisionSpace&& other) {
    if (this != &other) {
        _colliders = std::move(other._colliders);

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void CollisionSpace::init() {
    if (_initialized)
        throw InitializedException();
    
    _initialized = true;
}

void CollisionSpace::uninit() {
    if (!_initialized)
        return;

    _colliders.clear();
    _entitycolliderinfos.clear();
    _initialized = false;
}

void CollisionSpace::addCollider(EntityColliderInfo entitycolliderinfo, const char* name) {
    _entitycolliderinfos[name] = entitycolliderinfo;
}

EntityCollider* CollisionSpace::spawnCollider(const char* name, Entity* entity, Transform transform) {
    if (!entity)
        throw std::runtime_error("Attempt to spawn EntityCollider with null Entity reference");

    EntityCollider* collider = new EntityCollider;
    EntityColliderInfo &eci = _entitycolliderinfos[name];
    
    collider->_collisionspace = this;
    collider->_this_iter = _colliders.push_back(collider);
    collider->_filterstate.setFilter(&(eci.filter));
    collider->_entity = entity;

    collider->collision_enabled() = true;
    collider->_base_pos = eci.pos;
    collider->_base_scale = eci.scale;

    collider->resetTransformation();
    collider->applyTransform(transform);
    // force previous transformation to be current transformation
    collider->_prev_pos = collider->_cur_pos;
    collider->_prev_scale = collider->_cur_scale;

    return collider;
}

void CollisionSpace::erase(EntityCollider* collider) { _colliders.erase(collider->_this_iter); }

void CollisionSpace::detectCollisionAABB() {
    // perform pair-wise collision detection
    for (auto iter1 = _colliders.begin(); iter1 != _colliders.end(); iter1++) {

        // get Collider and skip if scale is zeroed out
        EntityCollider* c1 = *iter1;
        if (!(c1->_collision_enabled) || (c1->_cur_scale == glm::vec3(0.0f)))
            continue;

        auto iter2 = iter1;
        iter2++;
        for (;iter2 != _colliders.end(); iter2++) {

            // get other T and skip if scale is zeroed out (check t1's enable flag again in case it was unset this outer loop iteration)
            EntityCollider* c2 = *iter2;
            if (!(c2->_collision_enabled) || (c2->_cur_scale == glm::vec3(0.0f)))
                continue;

            // test filters against each other's IDs
            bool f1 = c1->_filterstate.hasFilter();
            bool f2 = c2->_filterstate.hasFilter();
            
            // if both have a filter, collide if both pass
            // if neither have a filter, collide
            // if only one has a filter, skip
            if (f1 != f2)
                continue;

            if (
                (!f1 && !f2) ||
                    (c1->_filterstate.pass(c2->_filterstate.id()) &&
                    c2->_filterstate.pass(c1->_filterstate.id()))
            ) {
                // detect and handle collision
                if (computeCollisionAABB({c1->_cur_pos, c1->_cur_scale}, {c2->_cur_pos, c2->_cur_scale})) {
                    // call collider on all of c1's attached scripts with c2's entity as arg
                    for (auto& es : c1->_scripts)
                        if (es != nullptr)
                            (*es)->collide(c2->_entity);
                    
                    // call collider on all of c2's attached scripts with c1's entity as arg
                    for (auto& es : c2->_scripts)
                        if (es != nullptr)
                            (*es)->collide(c1->_entity);
                }
            }

        }

    }
}

unsigned CollisionSpace::getCount() { return _colliders.size(); }

bool CollisionSpace::initialized() { return _initialized; }

// --------------------------------------------------------------------------------------------------------------------------

void EntityManager::_removeEntity(Entity* entity) {
    for (const unsigned &id : entity->_quad_ids)
        _glenv->remove(id);
    for (EntityCollider* collider : entity->_entitycolliders)
        _collisionspace->erase(collider);

    _entities[entity->_group.c_str()].erase(entity->_this_iter);
}

EntityManager::EntityManager(EntityScriptExecutor* entityscriptexecutor, GLEnv* glenv, CollisionSpace* collisionspace) : _initialized(false) { init(entityscriptexecutor, glenv, collisionspace); }
EntityManager::EntityManager(EntityManager&& other) { operator=(std::move(other)); }
EntityManager::EntityManager() : _entityscriptexecutor(nullptr), _glenv(nullptr), _collisionspace(nullptr), _initialized(false) {}
EntityManager::~EntityManager() { /* automatic destruction is fine */ }

EntityManager& EntityManager::operator=(EntityManager&& other) {
    if (this != &other) {
        _entityinfos = other._entityinfos;
        _entity_group_names = other._entity_group_names;

        // clear all lists
        std::vector<std::string> keys;
        for (std::unordered_map<std::string, ManagedList<Entity>>::iterator i = _entities.begin(); i != _entities.end(); ++i)
            keys.push_back(i->first);
        for (std::string s : keys)
            _entities.erase(s);

        // move all lists
        for (std::unordered_map<std::string, ManagedList<Entity>>::iterator i = other._entities.begin(); i != other._entities.end(); ++i)
            _entities[i->first] = std::move(i->second);

        _entityscriptexecutor = other._entityscriptexecutor;
        _glenv = other._glenv;
        _collisionspace = other._collisionspace;

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void EntityManager::init(EntityScriptExecutor* entityscriptexecutor, GLEnv* glenv, CollisionSpace* collisionspace) {
    if (_initialized)
        throw InitializedException();
    
    _entityscriptexecutor = entityscriptexecutor;
    _glenv = glenv;
    _collisionspace = collisionspace;
    _initialized = true;
}

void EntityManager::uninit() {
    if (!_initialized)
        return;
    
    _entityinfos.clear();
    _entity_group_names.clear();
    _entities.clear();
    _entityscriptexecutor = nullptr;
    _glenv = nullptr;
    _collisionspace = nullptr;
    _initialized = false;
}

void EntityManager::addEntity(EntityInfo info, const char* name) {
    // check if number of colliders matches number of attachment vectors
    if (info.entitycollider_names.size() != info.entitycollider_attachments.size())
        throw std::runtime_error("EntityInfo EntityCollider initializer list size is not equal to attachments initializer list size");
    
    // check if any index in attachment vector exceeds amount of entityscripts that will be instantiated with this EntityInfo
    for (auto &a : info.entitycollider_attachments)
        for (auto &i : a)
            if (i >= info.entityscript_names.size())
                throw std::runtime_error("EntityInfo has attachment index greater than EntityScript initializer list size");

    // insert info and create group if it does not exist
    _entityinfos[name] = info;
    if (_entities.find(info.group) == _entities.end()) {
        _entity_group_names.push_back(info.group);
        _entities[info.group] = ManagedList<Entity>();
    }
}

Entity* EntityManager::spawnEntity(const char* name, Transform transform) {
    EntityInfo& ei = _entityinfos[name];
    Entity* entity = new Entity();

    // reserve memory for each stored vector to guarantee memory addresses
    entity->_entityscripts.reserve(ei.entityscript_names.size());
    entity->_quad_ids.reserve(ei.quad_names.size());
    entity->_quads.reserve(ei.quad_names.size());
    entity->_entitycolliders.reserve(ei.entitycollider_names.size());

    // instantiate each EntityScriptInterface in info and push to entity's storage
    for (const std::string& esn : ei.entityscript_names) {
        entity->_entityscripts.push_back(_entityscriptexecutor->spawnEntityScript(esn.c_str(), entity));
        entity->_entityscripts.back()->attachNullableRef(&(entity->_entityscripts.back()));
    }

    // instantiate each Quad in info and push to entity's storage
    for (const std::string& qn : ei.quad_names) {
        entity->_quad_ids.push_back(_glenv->genQuad(qn.c_str(), transform));
        entity->_quads.push_back(_glenv->getQuad(entity->_quad_ids.back()));
    }

    // instantiate each EntityCollider in info and push to entity's storage (size match guaranteed by addEntity())
    auto ecn_iter = ei.entitycollider_names.begin();
    auto eca_iter = ei.entitycollider_attachments.begin();
    for (; ecn_iter != ei.entitycollider_names.end(); ecn_iter++, eca_iter++) {
        entity->_entitycolliders.push_back(_collisionspace->spawnCollider(ecn_iter->c_str(), entity, transform));

        // attach scripts
        for (auto &i : *eca_iter)
            entity->_entitycolliders.back()->attachNullableEntityScript(&(entity->_entityscripts[i]));
    }

    entity->_entity_name = name;
    entity->_this_iter = _entities[ei.group.c_str()].push_back(entity);
    entity->_entitymanager = this;
    entity->_globaltransform = transform;
    
    return entity;
}

void EntityManager::checkEntities() {
    std::queue<Entity*> remove_queue;

    // check script status of every entity in each group list
    for (auto &[name, mlist] : _entities)
        for (auto iter = mlist.begin(); iter != mlist.end(); ++iter) {
            auto& entity = *iter;

            // check if entity needs to be removed
            if (entity->getKillStarted()) {
                // check if all scripts nulled
                bool non_null_found = false;
                for (auto &es : entity->entityscripts()) {
                    if (es != nullptr) {
                        non_null_found = true;
                        break;
                    }
                }

                if (!non_null_found)
                    remove_queue.push(entity);
                
                continue;
            }

            // update transforms
            for (auto &q : entity->quads()) {
                q->resetTransformation();
                q->applyTransform(entity->globaltransform());
                
                q->animationstate().step();
                q->writeAnimation();
            }
            for (auto &ec : entity->entitycolliders()) {
                ec->resetTransformation();
                ec->applyTransform(entity->globaltransform());
            }
        }

    while (!remove_queue.empty()) {
        _removeEntity(remove_queue.front());
        remove_queue.pop();
    }
}

std::list<Entity*>::iterator EntityManager::groupBegin(const char *group) {
    if (_entities.find(group) == _entities.end())
        throw std::runtime_error(std::string("Attempt to get begin iterator of nonexistent group ") + group);
    return _entities[group].begin();
}

std::list<Entity*>::iterator EntityManager::groupEnd(const char *group) {
    if (_entities.find(group) == _entities.end())
        throw std::runtime_error(std::string("Attempt to get end iterator of nonexistent group ") + group);
    return _entities[group].end();
}

// --------------------------------------------------------------------------------------------------------------------------

bool computeCollisionAABB(Transform transf1, Transform transf2) {
    glm::vec3& pos1 = transf1.pos;
    glm::vec3& dim1 = transf1.scale;
    glm::vec3& pos2 = transf2.pos;
    glm::vec3& dim2 = transf2.scale;

    // get current collision
    float coll_x_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
    float coll_y_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
    float coll_z_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);

    if (coll_x_space < 0.0f && coll_y_space < 0.0f && coll_z_space < 0.0f)
        return true;
    return false;
}

glm::vec3 toVec3(glm::vec2 v, float z) { return glm::vec3(v.x, v.y, z); }

glm::vec2 toVec2(glm::vec3 v) { return glm::vec2(v.x, v.y); }

glm::vec3 randomAngle(glm::vec3 v, float deg_range) {
    if (deg_range == 0.0f)
        return v;
    return glm::rotate(v, glm::radians((-1.0f * deg_range) + float(rand() % int(deg_range * 2.0f))), glm::vec3(0.0f, 0.0f, 1.0f));
}

bool isEven(int x) {
    return (float(x) / 2.0f) == glm::floor(float(x) / 2.0f);
}