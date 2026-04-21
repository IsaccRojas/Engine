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
    if (_entity)
        _entity->checkScriptStatus();
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

// --------------------------------------------------------------------------------------------------------------------------

EntityScriptView::EntityScriptView(EntityScriptInterface* entityscript) : ScriptView(entityscript), _entityscript(entityscript) {}

void EntityScriptView::receive(Entity* entity, std::string message) { _entityscript->receive(entity, message); }

void EntityScriptView::collide(Entity* entity) { _entityscript->collide(entity); }

EntityScriptInterface* EntityScriptView::getEntityScript() { return _entityscript; }

// --------------------------------------------------------------------------------------------------------------------------

ScriptView EntityScriptExecutor::EntityScriptEnqueue::spawn() {
    return _entityscriptexecutor->spawnEntityScript(_name.c_str(), _execution_queue, _entity);
}

EntityScriptExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityScriptExecutor* entityscriptexecutor, std::string name, int execution_queue, Entity* entity) :
    ScriptEnqueue(nullptr, name, execution_queue), _entityscriptexecutor(entityscriptexecutor), _entity(entity)
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
        ScriptExecutor::addScript(ScriptInfo{entityscriptinfo._allocator, entityscriptinfo._spawn_callback, entityscriptinfo._remove_callback}, name);
        _entityscriptinfos[name] = entityscriptinfo;
    } else
        throw std::runtime_error("Attempt to add already added name");
}

EntityScriptView EntityScriptExecutor::spawnEntityScript(const char* entityscript_name, int execution_queue, Entity* entity) {
    // allocate instance and set it up
    EntityScriptInterface* entityscript = _entityscriptinfos[entityscript_name]._allocator->_allocate();
    _setupScript(entityscript, entityscript_name, execution_queue, _entityscriptinfos[entityscript_name]._allocator);
    _setupEntityScript(entityscript, entity);

    // run initialization method
    entityscript->runInit();

    return EntityScriptView(entityscript);
}

void EntityScriptExecutor::enqueueSpawnEntityScript(const char* entityscript_name, int execution_queue, Entity* entity) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, execution_queue, entity));
}

// --------------------------------------------------------------------------------------------------------------------------

Entity::Entity() : _entitymanager(nullptr), _entityscriptview(nullptr), _script_kill_started(false) {}
Entity::~Entity() {}

void Entity::checkScriptStatus() {
    _script_kill_started = _entityscriptview.getScript()->getKillStarted();
}

const char* Entity::getName() { return _entity_name.c_str(); }
std::vector<Quad*>& Entity::quads() { return _quads; }
std::vector<EntityColliderView>& Entity::entitycolliderviews() { return _entitycolliderviews; }
EntityScriptView& Entity::entityscriptview() { return _entityscriptview; }
Transform& Entity::globaltransform() { return _globaltransform; }

// --------------------------------------------------------------------------------------------------------------------------

EntityScriptAllocatorInterface::EntityScriptAllocatorInterface() {}

// --------------------------------------------------------------------------------------------------------------------------

EntityCollider::EntityCollider(EntityCollider&& other) { operator=(std::move(other)); }
EntityCollider::EntityCollider() :
    _collisionspace(nullptr),
    _collision_enabled(false),
    _base_pos(glm::vec3(0.0f)),
    _base_scale(glm::vec3(1.0f)),
    _pos(glm::vec3(0.0f)),
    _scale(glm::vec3(1.0f)),
    _prev_applied_transform(Transform{}),
    _entity(nullptr)
{}
EntityCollider::~EntityCollider() {}

EntityCollider& EntityCollider::operator=(EntityCollider&& other) {
    if (this != &other) {
        _collisionspace = other._collisionspace;
        _this_iter = other._this_iter;
        _filterstate = other._filterstate;
        _collision_enabled = other._collision_enabled;
        _base_pos = other._base_pos;
        _base_scale = other._base_scale;
        _pos = other._pos;
        _scale = other._scale;
        _prev_applied_transform = other._prev_applied_transform;
        _entity = other._entity;
        other._collisionspace = nullptr;
        other._collision_enabled = false;
        other._base_pos = glm::vec3(0.0f);
        other._base_scale = glm::vec3(1.0f);
        other._pos = glm::vec3(0.0f);
        other._scale = glm::vec3(1.0f);
        other._prev_applied_transform = Transform{};
        other._entity = nullptr;
    }
    return *this;
}

void EntityCollider::resetTransformation() {
    _pos = _base_pos;
    _scale = _base_scale;
}

void EntityCollider::applyTransform(Transform transform) {
    _prev_applied_transform = transform;
    _pos += transform.pos;
    _scale *= transform.scale;
}

Transform EntityCollider::getBaseTransformation() {
    return Transform{_base_pos, _base_scale};
}

Transform EntityCollider::getCurrentTransformation() {
    return Transform{_pos, _scale};
}

Transform EntityCollider::getPrevAppliedTransform() {
    return _prev_applied_transform;
}

FilterState& EntityCollider::filterstate() { return _filterstate; }
Entity& EntityCollider::entity() {return *_entity; }
bool& EntityCollider::collision_enabled() { return _collision_enabled; }

// --------------------------------------------------------------------------------------------------------------------------

EntityColliderView::EntityColliderView(EntityCollider* collider) : _collider(collider) {}

bool& EntityColliderView::collision_enabled() { return _collider->collision_enabled(); }

void EntityColliderView::resetTransformation() { _collider->resetTransformation(); }

void EntityColliderView::applyTransform(Transform transform) { _collider->applyTransform(transform); }

Transform EntityColliderView::getBaseTransformation() { return _collider->getBaseTransformation(); }

Transform EntityColliderView::getCurrentTransformation() { return _collider->getCurrentTransformation(); }

Transform EntityColliderView::getPrevAppliedTransform() { return _collider->getPrevAppliedTransform(); }

EntityCollider* EntityColliderView::getCollider() { return _collider; }

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

EntityColliderView CollisionSpace::spawnCollider(const char* name, Entity* entity, Transform transform) {
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

    return EntityColliderView(collider);
}

void CollisionSpace::erase(EntityColliderView colliderview) { _colliders.erase(colliderview.getCollider()->_this_iter); }

void CollisionSpace::detectCollisionAABB() {
    // perform pair-wise collision detection
    for (auto iter1 = _colliders.begin(); iter1 != _colliders.end(); iter1++) {

        // get Collider and skip if scale is zeroed out
        EntityCollider* c1 = *iter1;
        if (!(c1->collision_enabled()) || (c1->_scale == glm::vec3(0.0f)))
            continue;

        auto iter2 = iter1;
        iter2++;
        for (;iter2 != _colliders.end(); iter2++) {

            // get other T and skip if scale is zeroed out (check t1's enable flag again in case it was unset this outer loop iteration)
            EntityCollider* c2 = *iter2;
            if (!(c2->collision_enabled()) || (c2->_scale == glm::vec3(0.0f)))
                continue;

            // test filters against each other's IDs
            bool f1 = c1->filterstate().hasFilter();
            bool f2 = c2->filterstate().hasFilter();
            
            // if both have a filter, collide if both pass
            // if neither have a filter, collide
            // if only one has a filter, skip
            if (f1 != f2)
                continue;

            if (
                (!f1 && !f2) ||
                    (c1->filterstate().pass(c2->filterstate().id()) &&
                    c2->filterstate().pass(c1->filterstate().id()))
            ) {
                // detect and handle collision
                if (computeCollisionAABB({c1->_pos, c1->_scale}, {c2->_pos, c2->_scale})) {
                    c1->entity().entityscriptview().collide(&(c2->entity()));
                    c2->entity().entityscriptview().collide(&(c1->entity()));
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
    for (const EntityColliderView& view : entity->_entitycolliderviews)
        _collisionspace->erase(view);

    _entities[entity->_group.c_str()].erase(entity->_this_iter);
}

EntityManager::EntityManager(EntityScriptExecutor* entityscriptexecutor, GLEnv* glenv, CollisionSpace* collisionspace) : _initialized(false) { init(entityscriptexecutor, glenv, collisionspace); }
EntityManager::EntityManager(EntityManager&& other) { operator=(std::move(other)); }
EntityManager::EntityManager() : _entityscriptexecutor(nullptr), _glenv(nullptr), _collisionspace(nullptr), _initialized(false) {}
EntityManager::~EntityManager() { /* automatic destruction is fine */ }

EntityManager& EntityManager::operator=(EntityManager&& other) {
    if (this != &other) {
        _entityinfos = other._entityinfos;

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
    _entities.clear();
    _entityscriptexecutor = nullptr;
    _glenv = nullptr;
    _collisionspace = nullptr;
    _initialized = false;
}

void EntityManager::addEntity(EntityInfo info, const char* name) {
    _entityinfos[name] = info;
    _entities[name] = ManagedList<Entity>();
}

Entity* EntityManager::spawnEntity(const char* name, Transform transform) {
    EntityInfo& ei = _entityinfos[name];
    Entity* entity = new Entity();

    // instantiate each Quad in info and push to entity's storage
    for (const std::string& qn : ei.quad_names) {
        entity->_quad_ids.push_back(_glenv->genQuad(qn.c_str(), transform));
        entity->_quads.push_back(_glenv->getQuad(entity->_quad_ids.back()));
    }

    // instantiate each EntityCollider in info and push to entity's storage
    for (const std::string& ecn : ei.entitycollider_names)
        entity->_entitycolliderviews.push_back(_collisionspace->spawnCollider(ecn.c_str(), entity, transform));

    entity->_entity_name = name;
    entity->_this_iter = _entities[ei.group.c_str()].push_back(entity);
    entity->_entitymanager = this;
    entity->_globaltransform = transform;
    
    // instantiate EntityScriptInterface in info and push to entity's storage
    entity->_entityscriptview = _entityscriptexecutor->spawnEntityScript(ei.entityscript_name.c_str(), ei.execution_queue, entity);
    
    return entity;
}

void EntityManager::checkEntities() {
    std::queue<Entity*> remove_queue;

    // check script status of every entity in each group list
    for (auto &[name, mlist] : _entities)
        for (auto iter = mlist.begin(); iter != mlist.end(); ++iter) {
            auto& entity = *iter;

            // check if entity needs to be removed
            if (entity->_script_kill_started) {
                remove_queue.push(entity);
                continue;
            }

            // check if script needs to be enqueued
            EntityInfo &ei = _entityinfos[entity->getName()];
            if ((!(entity->entityscriptview().getExecEnqueued())) && ei.auto_enqueue)
                entity->entityscriptview().enqueueExec(ei.execution_queue);

            // update transforms
            for (auto &q : entity->quads()) {
                q->resetTransformation();
                q->applyTransform(entity->globaltransform());
                
                q->animationstate().step();
                q->writeAnimation();
            }
            for (auto &ecv : entity->entitycolliderviews()) {
                ecv.resetTransformation();
                ecv.applyTransform(entity->globaltransform());
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

glm::vec3 random_angle(glm::vec3 v, float deg_range) {
    if (deg_range == 0.0f)
        return v;
    return glm::rotate(v, glm::radians((-1.0f * deg_range) + float(rand() % int(deg_range * 2.0f))), glm::vec3(0.0f, 0.0f, 1.0f));
}

bool is_even(int x) {
    return (float(x) / 2.0f) == glm::floor(float(x) / 2.0f);
}