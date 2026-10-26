#include "../include/entity.hpp"

EntityInterface::EntityInterface(EntityInterface&& other) {
    operator=(std::move(other));
}
EntityInterface::EntityInterface(EntityInterface* root, EntityInterface* parent) : 
    ScriptInterface(), _root(root), _parent(parent), _entity_name(""), _entityexecutor(nullptr), _entity_kill_started(false), _globaltransform(nullptr)
{}
EntityInterface::EntityInterface() : 
    ScriptInterface(), _root(nullptr), _parent(nullptr), _entity_name(""), _entityexecutor(nullptr), _entity_kill_started(false), _globaltransform(nullptr)
{}
EntityInterface::~EntityInterface() {}

EntityInterface& EntityInterface::operator=(EntityInterface&& other) {
    if (this != &other)
        ScriptInterface::operator=(std::move(other));
    return *this;
}

void EntityInterface::_init() {
    // lock parent to prevent kill before children are killed
    if (_parent)
        _parent->lock(&key());

    _initEntity();
}

void EntityInterface::_exec() {
    _execEntity();
}

void EntityInterface::_kill() {
    _killEntity();

    // null all inserted nullable references
    for (EntityInterface** r : _nullablerefs)
        *r = nullptr;
    
    if (_parent)
        _parent->unlock(&key());
    
    _entityexecutor->eraseEntityResources(this);
}

void EntityInterface::_initEntity() {}
void EntityInterface::_execEntity() {}
void EntityInterface::_killEntity() {}
void EntityInterface::_collideEntity(EntityInterface* entity) {}

void EntityInterface::collide(EntityInterface* entity) {
    _collideEntity(entity);
}

void EntityInterface::attachNullableRef(EntityInterface** ref) {
    _nullablerefs.insert(ref);
}
void EntityInterface::detachNullableRef(EntityInterface** ref) {
    _nullablerefs.erase(ref);
}

void EntityInterface::enqueueKillAll() {
    if (!_entity_kill_started) {
        for (auto& e : _entities)
            if (e != nullptr)
                if (!e->_entity_kill_started)
                    e->enqueueKillAll();

        enqueueKill();
        _entity_kill_started = true;
    }
}

std::vector<EntityInterface*>& EntityInterface::entities() { return _entities; }
std::vector<EntityInterface*>& EntityInterface::entitiesflat() { return _entities_flat; }
std::vector<Quad*>& EntityInterface::quads() { return _quads; }
std::vector<EntityCollider*>& EntityInterface::entitycolliders() { return _entitycolliders; }

Transform& EntityInterface::globaltransform() { return *_globaltransform; }
Transform EntityInterface::getPrevGlobalTransform() { return _prevglobaltransform; }

const char* EntityInterface::getName() { return _entity_name.c_str(); }
const char* EntityInterface::getGroup() { return _group.c_str(); }
bool EntityInterface::getEntityKillStarted() { return _entity_kill_started; }

EntityInterface* EntityInterface::root() { return _root; }
EntityInterface* EntityInterface::parent() { return _parent; };

// --------------------------------------------------------------------------------------------------------------------------

ScriptInterface* EntityExecutor::EntityEnqueue::spawn() {
    return _entityexecutor->spawnEntity(_name.c_str(), _transform);
}

EntityExecutor::EntityEnqueue::EntityEnqueue(EntityExecutor* entityexecutor, std::string name, Transform transform) :
    ScriptEnqueue(nullptr, name, false), _entityexecutor(entityexecutor), _transform(transform)
{}

EntityExecutor::EntityExecutor(unsigned queues, GLEnv* glenv, CollisionSpace* collisionspace) : ScriptExecutor(queues), _glenv(glenv), _collisionspace(collisionspace) {}
EntityExecutor::EntityExecutor(EntityExecutor &&other) : ScriptExecutor() { operator=(std::move(other)); }
EntityExecutor::~EntityExecutor() { /* automatic destruction is fine */ }

EntityExecutor& EntityExecutor::operator=(EntityExecutor&& other) {
    if (this == &other) {
        ScriptExecutor::operator=(std::move(other));
        _entityinfos = other._entityinfos;
        other._entityinfos.clear();
    }
    return *this;
}

void EntityExecutor::eraseEntityResources(EntityInterface* entity) {
    for (const unsigned &id : entity->_quad_ids)
        _glenv->remove(id);
    for (EntityCollider* collider : entity->_entitycolliders)
        _collisionspace->erase(collider);

    if (!entity->_parent)
        delete entity->_globaltransform;

    _entity_refs.erase(entity);
}

EntityInterface* EntityExecutor::_spawnEntityNode(const char* entity_name, EntityInterface* root, EntityInterface* parent, Transform transform) {
    auto ei_iter = _entityinfos.find(entity_name);
    if (ei_iter == _entityinfos.end())
        throw std::runtime_error((std::string("Attempt to spawn Entity with non-existent EntityInfo name '") + entity_name) + std::string("'"));
    EntityInfo& ei = ei_iter->second;

    // allocate instance and set it up
    EntityInterface* entity = ei.allocator->_allocate();
    _setupScript(entity, entity_name, ei.allocator);
    
    // if no parent, allocate transform; else, use parent's
    Transform* active_transform;
    if (!parent) {
        entity->_globaltransform = new Transform(transform);
        entity->_prevglobaltransform = transform;
        entity->_root = entity;
        entity->_parent = nullptr;

        active_transform = entity->_globaltransform;
    }
    else {
        entity->_globaltransform = parent->_globaltransform;
        entity->_prevglobaltransform = parent->_prevglobaltransform;
        entity->_root = root;
        entity->_parent = parent;

        active_transform = parent->_globaltransform;
    }

    // reserve memory for each stored vector to guarantee memory addresses
    entity->_quad_ids.reserve(ei.quad_names.size());
    entity->_quads.reserve(ei.quad_names.size());
    entity->_entitycolliders.reserve(ei.entitycollider_names.size());
    entity->_entities.resize(ei.entity_names.size());

    // instantiate each Quad in info and push to entity's storage
    for (const std::string& qn : ei.quad_names) {
        entity->_quad_ids.push_back(_glenv->genQuad(qn.c_str(), *active_transform));
        entity->_quads.push_back(_glenv->getQuad(entity->_quad_ids.back()));
    }
    
    // instantiate each EntityCollider in info and push to entity's storage (attaches nullable ref internally)
    for (const std::string& ecn : ei.entitycollider_names)
        entity->_entitycolliders.push_back(_collisionspace->spawnEntityCollider(ecn.c_str(), entity, *active_transform));

    // initialize fields before running init methods
    entity->_entity_name = entity_name;
    entity->_entityexecutor = this;

    // instantiate each entity in info and push to storage
    auto esn_iter = ei.entity_names.begin();
    int i = 0;
    for (; esn_iter != ei.entity_names.end(); esn_iter++, i++) {
        if (!parent) {
            entity->_entities[i] = _spawnEntityNode(esn_iter->c_str(), entity, entity, transform);
            entity->_entities_flat.push_back(entity->_entities[i]);
        }
        else {
            entity->_entities[i] = _spawnEntityNode(esn_iter->c_str(), root, entity, transform);
            root->_entities_flat.push_back(entity->_entities[i]);
        }

        // attach entries in local non-flattened storage as nullable refs
        entity->_entities[i]->attachNullableRef(&(entity->_entities[i]));
    }

    // attach entries in root's flattened storage as nullable refs
    if (!parent) {
        entity->_entities_flat.push_back(entity);
        for (i = 0; i < entity->_entities_flat.size(); i++)
            entity->_entities_flat[i]->attachNullableRef(&entity->_entities_flat[i]);
    }

    ei.allocator->_onInitialization(entity);
    entity->runInit();

    if (ei.spawn_callback)
        ei.spawn_callback(entity);

    _entity_refs.insert(entity);
    return entity;
}

void EntityExecutor::addEntity(EntityInfo info, const char* name) {
    std::list<std::string> entity_names_flat;

    // check names
    if (hasAdded(name))
        throw std::runtime_error((std::string("Attempt to add existing EntityInfo name '") + name) + std::string("'"));
    for (auto &i: info.quad_names)
        if (!_glenv->hasAdded(i.c_str()))
            throw std::runtime_error(
                (std::string("Attempt to add EntityInfo name '") + name) 
                + std::string("'") 
                + (std::string(" with non-existent QuadInfo name '") + i) 
                + std::string("'")
            );
    for (auto &i: info.entitycollider_names)
        if (!_collisionspace->hasAdded(i.c_str()))
            throw std::runtime_error(
                (std::string("Attempt to add EntityInfo name '") + name) 
                + std::string("'") 
                + (std::string(" with non-existent EntityColliderInfo name '") + i) 
                + std::string("'")
            );

    for (auto &i: info.entity_names) {
        if (!hasAdded(i.c_str()))
            throw std::runtime_error(
                (std::string("Attempt to add EntityInfo name '") + name) 
                + std::string("'") 
                + (std::string(" with non-existent contained EntityInfo name '") + i) 
                + std::string("'")
            );
    
        // flatten existing EntityInfo names into this one
        entity_names_flat.insert(entity_names_flat.end(), _entityinfos[i].entity_names.begin(), _entityinfos[i].entity_names.end());
        entity_names_flat.insert(entity_names_flat.end(), i);
    }

    // check if duplicate EntityInfo name
    std::string s = hasDuplicate(entity_names_flat);
    if (s != "")
        throw std::runtime_error(
            (std::string("Attempt to add EntityInfo name '") + name) 
            + std::string("'") 
            + (std::string(" with multiple contained EntityInfo names '") + s) 
            + std::string("'")
        );

    // insert info and create group if it does not exist
    ScriptExecutor::addScript(ScriptInfo{
        info.allocator, 
        info.preferred_queue, 
        info.auto_enqueue, 
        info.spawn_callback, 
        info.remove_callback
        }, name);
    _entityinfos[name] = info;
}

void EntityExecutor::enqueueSpawnEntity(const char* entity_name, Transform transform) {
    _pushSpawnEnqueue(new EntityEnqueue(this, entity_name, transform));
}

EntityInterface* EntityExecutor::spawnEntity(const char* entity_name, Transform transform) {
    auto ei_iter = _entityinfos.find(entity_name);
    if (ei_iter == _entityinfos.end())
        throw std::runtime_error((std::string("Attempt to spawn Entity with non-existent EntityInfo name '") + entity_name) + std::string("'"));

    return _spawnEntityNode(entity_name, nullptr, nullptr, transform);
}

void EntityExecutor::update() {
    // check script status of every entity in each group list
    for (auto& entity : _entity_refs) {
        // update transforms
        entity->_prevglobaltransform = entity->globaltransform();
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
}

// --------------------------------------------------------------------------------------------------------------------------

EntityAllocatorInterface::EntityAllocatorInterface() {}

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

FilterState& EntityCollider::filterstate() { return _filterstate; }
bool& EntityCollider::collision_enabled() { return _collision_enabled; }

// --------------------------------------------------------------------------------------------------------------------------

CollisionSpace::CollisionSpace() {}
CollisionSpace::CollisionSpace(CollisionSpace&& other) { operator=(std::move(other)); }
CollisionSpace::~CollisionSpace() { /* automatic destruction is fine */ }

CollisionSpace& CollisionSpace::operator=(CollisionSpace&& other) {
    if (this != &other) {
        _colliders = std::move(other._colliders);
        other._colliders.clear();
    }
    return *this;
}

void CollisionSpace::addEntityCollider(EntityColliderInfo entitycolliderinfo, const char* name) {
    if (hasAdded(name))
        throw std::runtime_error((std::string("Attempt to add existing EntityColliderInfo name '") + name) + std::string("'"));
    _entitycolliderinfos[name] = entitycolliderinfo;
}

EntityCollider* CollisionSpace::spawnEntityCollider(const char* entitycollider_name, EntityInterface* entity, Transform transform) {
    auto eci_iter = _entitycolliderinfos.find(entitycollider_name);
    if (eci_iter == _entitycolliderinfos.end())
        throw std::runtime_error((std::string("Attempt to spawn EntityCollider with non-existent EntityColliderInfo name '") + entitycollider_name) + std::string("'"));

    EntityCollider* collider = new EntityCollider;
    EntityColliderInfo &eci = eci_iter->second;
    
    collider->_collisionspace = this;
    collider->_this_iter = _colliders.push_back(collider);
    collider->_filterstate.setFilter(&(eci.filter));

    collider->collision_enabled() = true;
    collider->_base_pos = eci.pos;
    collider->_base_scale = eci.scale;

    collider->resetTransformation();
    collider->applyTransform(transform);
    // force previous transformation to be current transformation
    collider->_prev_pos = collider->_cur_pos;
    collider->_prev_scale = collider->_cur_scale;

    collider->_entity = entity;
    entity->attachNullableRef(&collider->_entity);

    return collider;
}

void CollisionSpace::erase(EntityCollider* collider) { _colliders.erase(collider->_this_iter); }

void CollisionSpace::detectCollisionAABB() {
    // perform pair-wise collision detection
    for (auto iter1 = _colliders.begin(); iter1 != _colliders.end(); iter1++) {

        // get Collider and skip if entity is invalid or if scale is zeroed out
        EntityCollider* c1 = *iter1;

        if (!c1->_entity || c1->_entity->getKillEnqueued())
            continue;
        if (!(c1->_collision_enabled) || (c1->_cur_scale == glm::vec3(0.0f)))
            continue;

        auto iter2 = iter1;
        iter2++;
        for (;iter2 != _colliders.end(); iter2++) {

            // get other T and skip if entity is invalid or if scale is zeroed out
            EntityCollider* c2 = *iter2;
            if (!c2->_entity || c2->_entity->getKillEnqueued())
                continue;
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
                    // call collision methods on all entities in collider's attached entity tree, with other collider's attached entity as arg
                    for (auto& e : c1->_entity->root()->entitiesflat())
                        if (e && !e->getKillEnqueued())
                            e->collide(c2->_entity);

                    for (auto& e : c2->_entity->root()->entitiesflat())
                        if (e && !e->getKillEnqueued())
                            e->collide(c1->_entity);
                }
            }

        }

    }
}

bool CollisionSpace::hasAdded(const char* collider_name) { return !(_entitycolliderinfos.find(collider_name) == _entitycolliderinfos.end()); }

unsigned CollisionSpace::getCount() { return _colliders.size(); }