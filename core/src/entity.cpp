#include "../include/entity.hpp"

EntityInterface::EntityInterface(EntityInterface&& other) {
    operator=(std::move(other));
}
EntityInterface::EntityInterface(EntityInterface* parent) : 
    ScriptInterface(), _parent(parent), _entity_name(""), _entityexecutor(nullptr), _kill_started(false), _globaltransform(nullptr)
{}
EntityInterface::~EntityInterface() {}

EntityInterface& EntityInterface::operator=(EntityInterface&& other) {
    if (this != &other)
        ScriptInterface::operator=(std::move(other));
    return *this;
}

void EntityInterface::_init() {
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
}

void EntityInterface::_initEntity() {}
void EntityInterface::_execEntity() {}
void EntityInterface::_killEntity() {}
void EntityInterface::_receive(EntityInterface* entity, std::string message) {}
void EntityInterface::_collide(EntityInterface* entity) {}
void EntityInterface::receive(EntityInterface* entity, std::string message) {
    _receive(entity, message);
}
void EntityInterface::collide(EntityInterface* entity) {
    _collide(entity);
}

void EntityInterface::attachNullableRef(EntityInterface** ref) {
    _nullablerefs.insert(ref);
}
void EntityInterface::detachNullableRef(EntityInterface** ref) {
    _nullablerefs.erase(ref);
}

void EntityInterface::startKill() {
    if (!_kill_started) {
        for (auto& e : _entities)
            if (e != nullptr)
                if (!e->_kill_started)
                    e->startKill();

        _kill_started = true;
    }
}

void EntityInterface::startKillAll() {
    if (_parent)
        _parent->startKillAll();
    else
        startKill();
}



std::vector<EntityInterface*>& EntityInterface::entities() { return _entities; }
std::vector<Quad*>& EntityInterface::quads() { return _quads; }
std::vector<EntityCollider*>& EntityInterface::entitycolliders() { return _entitycolliders; }

Transform& EntityInterface::globaltransform() { return *_globaltransform; }
Transform EntityInterface::getPrevGlobalTransform() { return _prevglobaltransform; }

const char* EntityInterface::getName() { return _entity_name.c_str(); }
const char* EntityInterface::getGroup() { return _group.c_str(); }
bool EntityInterface::getKillStarted() { return _kill_started; }

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

void EntityExecutor::_removeEntity(EntityInterface* entity) {
    for (const unsigned &id : entity->_quad_ids)
        _glenv->remove(id);
    for (EntityCollider* collider : entity->_entitycolliders)
        _collisionspace->erase(collider);

    if (!entity->_parent)
        delete entity->_globaltransform;

    entity->enqueueKill();
    _entity_refs.erase(entity);
}

EntityInterface* EntityExecutor::_spawnEntityNode(const char* entity_name, EntityInterface* parent, Transform transform) {
    auto ei_iter = _entityinfos.find(entity_name);
    if (ei_iter == _entityinfos.end())
        throw std::runtime_error((std::string("Attempt to spawn Entity with non-existent EntityInfo name '") + entity_name) + std::string("'"));
    EntityInfo& ei = ei_iter->second;

    // allocate instance and set it up
    EntityInterface* entity = ei_iter->second.allocator->_allocate();
    _setupScript(entity, entity_name, ei_iter->second.allocator);
    entity->_parent = parent;
    
    // if no parent, allocate transform; else, use parent's
    Transform* active_transform;
    if (!parent) {
        entity->_globaltransform = new Transform(transform);
        entity->_prevglobaltransform = transform;
        active_transform = entity->_globaltransform;
    }
    else {
        entity->_globaltransform = parent->_globaltransform;
        entity->_prevglobaltransform = parent->_prevglobaltransform;
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
    
    // instantiate each EntityCollider in info and push to entity's storage
    for (const std::string& ecn : ei.entitycollider_names)
        entity->_entitycolliders.push_back(_collisionspace->spawnEntityCollider(ecn.c_str(), entity, *active_transform));

    // initialize fields before running init methods
    entity->_entity_name = entity_name;
    entity->_entityexecutor = this;
    _entity_refs.insert(entity);

    // instantiate each entity in info and push to storage
    auto esn_iter = ei.entity_names.begin();
    int i = 0;
    for (; esn_iter != ei.entity_names.end(); esn_iter++, i++) {
        if (!parent)
            entity->_entities[i] = _spawnEntityNode(esn_iter->c_str(), entity, transform);
        else
            entity->_entities[i] = _spawnEntityNode(esn_iter->c_str(), parent, transform);
        
        entity->_entities[i]->attachNullableRef(&(entity->_entities[i]));
    }

    for (auto& es : entity->_entities)
        es->runInit();

    if (ei.spawn_callback)
        ei.spawn_callback(entity);

    return entity;
}

void EntityExecutor::addEntity(EntityInfo info, const char* name) {
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
    for (auto &i: info.entity_names)
        if (!hasAdded(i.c_str()))
            throw std::runtime_error(
                (std::string("Attempt to add EntityInfo name '") + name) 
                + std::string("'") 
                + (std::string(" with non-existent EntityScriptInfo name '") + i) 
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

    return _spawnEntityNode(entity_name, nullptr, transform);
}

void EntityExecutor::update() {
    std::queue<EntityInterface*> remove_queue;

    // check script status of every entity in each group list
    for (auto& entity : _entity_refs) {
        // check if entity needs to be removed
        if (entity->getKillStarted()) {
            // check if all scripts nulled
            bool non_null_found = false;
            for (auto &es : entity->entities()) {
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

    while (!remove_queue.empty()) {
        _removeEntity(remove_queue.front());
        remove_queue.pop();
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
                    // call collision method on collider's attached entity with other collider's attached entity as arg
                    c1->_entity->collide(c2->_entity);
                    c2->_entity->collide(c1->_entity);
                }
            }

        }

    }
}

bool CollisionSpace::hasAdded(const char* collider_name) { return !(_entitycolliderinfos.find(collider_name) == _entitycolliderinfos.end()); }

unsigned CollisionSpace::getCount() { return _colliders.size(); }

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

float computeDistanceAABB(Transform transf1, Transform transf2, glm::bvec3 axis) {
    glm::vec3& pos1 = transf1.pos;
    glm::vec3& dim1 = transf1.scale;
    glm::vec3& pos2 = transf2.pos;
    glm::vec3& dim2 = transf2.scale;

    // get current collision
    if (axis.x) {
        float coll_x_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
        return coll_x_space;
    }

    if (axis.y) {
        float coll_y_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
        return coll_y_space;
    }
    
    if (axis.z) {
        float coll_z_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);
        return coll_z_space;
    }

    return 0.0f;
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

float absMin(float a, float b) {
    return (glm::abs(a) >= glm::abs(b)) ? b : a;
}