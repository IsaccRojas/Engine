#include "../include/entity.hpp"

EntityScript::EntityScript(EntityScript &&other) {
    operator=(std::move(other));
    _entity = other._entity;
    other._entity = nullptr;
}
EntityScript::EntityScript() : 
    Script(), _entity(nullptr)
{}
EntityScript::~EntityScript() { /* automatic destruction is fine */ }

EntityScript &EntityScript::operator=(EntityScript &&other) {
    if (this != &other) {
        Script::operator=(std::move(other));
        _entity = other._entity;
        other._entity = nullptr;
    }
    return *this;
}

void EntityScript::_init() {
    _initEntity();
}

void EntityScript::_exec() {
    _execEntity();
}

void EntityScript::_kill() {
    _killEntity();
    if (_entity)
        _entity->_script_killed = true;
}

void EntityScript::_update() {
    _updateEntity();
}

Entity &EntityScript::entity() {
    return *_entity;
};

bool EntityScript::hasEntity() {
    return !(_entity == nullptr);
}

void EntityScript::receive(Entity *entity, std::string message) {
    _receive(entity, message);
}

// --------------------------------------------------------------------------------------------------------------------------

EntityScriptView::EntityScriptView(EntityScript *entityscript) : ScriptView(entityscript), _entityscript(entityscript) {}
void EntityScriptView::receive(Entity *entity, std::string message) {
    _entityscript->receive(entity, message);
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptView EntityExecutor::EntityScriptEnqueue::spawn() {
    return _entityexecutor->spawnEntityScript(_name.c_str(), _execution_queue, _entity);
}

EntityExecutor::EntityScriptEnqueue::EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, Entity *entity) :
    ScriptEnqueue(nullptr, name, execution_queue), _entityexecutor(entityexecutor), _entity(entity)
{}

void EntityExecutor::_setupEntityScript(EntityScript *entityscript, Entity *entity) {
    entityscript->_entity = entity;
}

EntityExecutor::EntityExecutor(unsigned queues) : Executor() { 
    init(queues);
}
EntityExecutor::EntityExecutor(EntityExecutor &&other) : Executor() { operator=(std::move(other)); }
EntityExecutor::EntityExecutor() : Executor() {}
EntityExecutor::~EntityExecutor() { /* automatic destruction is fine */ }

EntityExecutor &EntityExecutor::operator=(EntityExecutor &&other) {
    if (this == &other) {
        Executor::operator=(std::move(other));
        _entityscriptinfos = other._entityscriptinfos;
        other._entityscriptinfos.clear();
    }
    return *this;
}

void EntityExecutor::init(unsigned queues) {
    Executor::init(queues);
}

void EntityExecutor::uninit() {
    Executor::uninit();
    _entityscriptinfos.clear();
}

void EntityExecutor::addEntityScript(EntityScriptAllocatorInterface *allocator, const char *name, std::function<void(ScriptView)> spawn_callback, std::function<void(ScriptView)> remove_callback) {
    if (!hasAdded(name)) {
        Executor::add(nullptr, name, spawn_callback, remove_callback);
        _entityscriptinfos[name] = EntityScriptInfo{allocator};
    } else
        throw std::runtime_error("Attempt to add already added name");
}

EntityScriptView EntityExecutor::spawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity) {
    // allocate instance and set it up
    EntityScript *entityscript = _entityscriptinfos[entityscript_name]._allocator->_allocate();
    _setupScript(entityscript, entityscript_name, execution_queue);
    _setupEntityScript(entityscript, entity);

    // run initialization method
    entityscript->runInit();

    return EntityScriptView(entityscript);
}

void EntityExecutor::enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity) {
    _pushSpawnEnqueue(new EntityScriptEnqueue(this, entityscript_name, execution_queue, entity));
}

// --------------------------------------------------------------------------------------------------------------------------

Entity::Entity() : _entitymanager(nullptr), _entityscriptview(nullptr), _script_killed(false) {}
Entity::~Entity() {}

EntityManager &Entity::manager() { return *_entitymanager; }
std::vector<Quad*> &Entity::quads() { return _quads; }
std::vector<EntityColliderView> &Entity::entitycolliderviews() { return _entitycolliderviews; }
std::unordered_map<std::string, float> &Entity::attributes1f() { return _attributes1f; }
std::unordered_map<std::string, glm::vec2> &Entity::attributes2f() { return _attributes2f; }
std::unordered_map<std::string, glm::vec3> &Entity::attributes3f() { return _attributes3f; }
EntityScriptView &Entity::entityscriptview() { return _entityscriptview; }

// --------------------------------------------------------------------------------------------------------------------------

EntityCollider::EntityCollider(EntityCollider &&other) { operator=(std::move(other)); }
EntityCollider::EntityCollider() :
    _collisionspace(nullptr),
    _collision_enabled(false)
{}
EntityCollider::~EntityCollider() {}

EntityCollider& EntityCollider::operator=(EntityCollider &&other) {
    if (this != &other) {
        _collisionspace = other._collisionspace;
        _this_iter = other._this_iter;
        _filterstate = other._filterstate;
        _collision_enabled = other._collision_enabled;
        other._collisionspace = nullptr;
        other._collision_enabled = false;
    }
    return *this;
}

FilterState& EntityCollider::filterstate() { return _filterstate; }

void EntityCollider::step() { transform.pos += vel; }

// --------------------------------------------------------------------------------------------------------------------------

EntityColliderView::EntityColliderView(EntityCollider *collider) : _collider(collider) {}

Transform& EntityColliderView::transform() { return _collider->transform; }

// --------------------------------------------------------------------------------------------------------------------------

CollisionSpace::CollisionSpace(unordered_map_string_Filter_t *filters) { init(filters); }
CollisionSpace::CollisionSpace() : _filters(nullptr), _initialized(false) {}
CollisionSpace::CollisionSpace(CollisionSpace &&other) { operator=(std::move(other)); }
CollisionSpace::~CollisionSpace() { /* automatic destruction is fine */ }

CollisionSpace &CollisionSpace::operator=(CollisionSpace &&other) {
    if (this != &other) {
        _colliders = std::move(other._colliders);

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void CollisionSpace::init(unordered_map_string_Filter_t *filters) {
    if (_initialized)
        throw InitializedException();
    
    _filters = filters;
    _initialized = true;
}

void CollisionSpace::uninit() {
    if (!_initialized)
        return;

    _colliders.clear();
    _filters = nullptr;
    _initialized = false;
}

EntityColliderView CollisionSpace::spawnCollider(Transform transform, glm::vec3 vel, const char *filter_name) {
    EntityCollider *collider = new EntityCollider;
    
    collider->_collisionspace = this;
    collider->_this_iter = _colliders.push_back(collider);
    collider->_filterstate.setFilter(&(*_filters)[filter_name]);
    collider->_collision_enabled = true;

    return EntityColliderView(collider);
}

void CollisionSpace::erase(EntityColliderView colliderview) { _colliders.erase(colliderview._collider->_this_iter); }

void CollisionSpace::detectCollisionAABB() {
    // perform pair-wise collision detection
    for (auto iter1 = _colliders.begin(); iter1 != _colliders.end(); iter1++) {

        // get Collider and skip if scale is zeroed out
        EntityCollider *c1 = *iter1;
        if (!(c1->_collision_enabled) || (c1->transform.scale == glm::vec3(0.0f)))
            continue;

        auto iter2 = iter1;
        iter2++;
        for (;iter2 != _colliders.end(); iter2++) {

            // get other T and skip if scale is zeroed out (check t1's enable flag again in case it was unset this outer loop iteration)
            EntityCollider *c2 = *iter2;
            if (!(c1->_collision_enabled) || !(c2->_collision_enabled) || (c2->transform.scale == glm::vec3(0.0f)))
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
                if (computeCollisionAABB(c1->transform, c2->transform)) {
                    // TODO: do something
                }
            }

        }

    }
}

void CollisionSpace::step() {
    for (auto iter = _colliders.begin(); iter != _colliders.end(); iter++)
        (*iter)->step();
}

unsigned CollisionSpace::getCount() { return _colliders.size(); }

bool CollisionSpace::initialized() { return _initialized; }

// --------------------------------------------------------------------------------------------------------------------------

void EntityManager::_removeEntity(Entity *entity) {
    for (const unsigned &id : entity->_quad_ids)
        _glenv->remove(id);
    for (const EntityColliderView &view : entity->_entitycolliderviews)
        _collisionspace->erase(view);

    _entities[entity->_group.c_str()].erase(entity->_this_iter);
}

EntityManager::EntityManager(EntityExecutor *entityexecutor, GLEnv *glenv, CollisionSpace *collisionspace) : _initialized(false) { init(entityexecutor, glenv, collisionspace); }
EntityManager::EntityManager(EntityManager &&other) { operator=(std::move(other)); }
EntityManager::EntityManager() : _entityexecutor(nullptr), _glenv(nullptr), _collisionspace(nullptr), _initialized(false) {}
EntityManager::~EntityManager() { /* automatic destruction is fine */ }

EntityManager &EntityManager::operator=(EntityManager &&other) {
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

        _entityexecutor = other._entityexecutor;
        _glenv = other._glenv;
        _collisionspace = other._collisionspace;

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void EntityManager::init(EntityExecutor *entityexecutor, GLEnv *glenv, CollisionSpace *collisionspace) {
    if (_initialized)
        throw InitializedException();
    
    _entityexecutor = entityexecutor;
    _glenv = glenv;
    _collisionspace = collisionspace;
    _initialized = true;
}

void EntityManager::uninit() {
    if (!_initialized)
        return;
    
    _entityinfos.clear();
    _entities.clear();
    _entityexecutor = nullptr;
    _glenv = nullptr;
    _collisionspace = nullptr;
    _initialized = false;
}

void EntityManager::addEntity(EntityInfo info, const char *name) {
    _entityinfos[name] = info;
    _entities[name] = ManagedList<Entity>();
}

Entity *EntityManager::spawnEntity(const char *name) {
    EntityInfo &ei = _entityinfos[name];
    Entity *entity = new Entity();

    // instantiate each Quad in info and push to entity's storage
    for (const QuadArgs &a : ei._quad_args) {
        entity->_quad_ids.push_back(_glenv->genQuad(a.pos, a.scale, a.color, a.type, a.animation_name.c_str(), a.texpos, a.texsize, a.innerrad));
        entity->_quads.push_back(_glenv->getQuad(entity->_quad_ids.back()));
    }

    // instantiate each Box in info and push to entity's storage
    for (const EntityColliderArgs &a : ei._entitycollider_args)
        entity->_entitycolliderviews.push_back(_collisionspace->spawnCollider(a.transf, a.vel, a.filter_name.c_str()));

    // instantiate each EntityScript in info and push to entity's storage
    entity->_entityscriptview = _entityexecutor->spawnEntityScript(ei._entityscript_args.entityscript_name.c_str(), ei._entityscript_args.execution_queue, entity);

    entity->_this_iter = _entities[ei._group.c_str()].push_back(entity);
    entity->_entitymanager = this;
    
    return entity;
}

void EntityManager::checkEntities() {
    std::queue<Entity*> remove_queue;

    // check every script status of every entity in each group list
    for (auto &[name, mlist] : _entities)
        for (auto iter = mlist.begin(); iter != mlist.end(); ++iter)
            if ((*iter)->_script_killed)
                remove_queue.push(*iter);

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
    glm::vec3 &pos1 = transf1.pos;
    glm::vec3 &dim1 = transf1.scale;
    glm::vec3 &pos2 = transf2.pos;
    glm::vec3 &dim2 = transf2.scale;

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