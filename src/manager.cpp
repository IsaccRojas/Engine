#include "manager.hpp"

Manager::Manager(int maxcount) :
    _maxcount(maxcount),
    _executor(nullptr),
    _glenv(nullptr),
    _animations(nullptr),
    _collider(nullptr),
    _managerptr(nullptr)
{
    for (int i = 0; i < maxcount; i++) {
        _scripts.push_back(std::unique_ptr<Script>(nullptr));
        _scriptvalues.push_back(_ScriptValues{MNG_TYPE_NONE, -1, -1, -1, NULL, nullptr, nullptr});
    }
    _managerptr = new ManagerPtr(this);
}
Manager::~Manager() {
    // iterate on existing ids
    auto ids = _ids.getused();
    for (int i = 0; i < ids.size(); i++) {
        
        _ScriptValues &values = _scriptvalues[ids[i]];
        // remove from any existing structures corresponding to its type
        switch (values._type) {
            case MNG_TYPE_ENTITY:
                removeEntity(values._manager_id);
                    
            case MNG_TYPE_OBJECT:
                removeObject(values._manager_id);

            default: // should never occur
                break;
            
        }
    }
    delete _managerptr;
}

bool Manager::hasEntity(const char *entityname) { return !(_entitytypes.find(entityname) == _entitytypes.end()); }

bool Manager::hasObject(const char *objectname) { return !(_objecttypes.find(objectname) == _objecttypes.end()); }

int Manager::spawnEntity(const char *entityname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount)
        return -1;

    _EntityType &type = _entitytypes[entityname];
    Entity *entity = type._allocator();
    
    // set up entity
    if (type._force_scriptsetup)
        if (_executor)
            entity->scriptSetup(_executor);
    if (type._force_entitysetup)
        if (_glenv && _animations)
            entity->entitySetup(_glenv, &(*_animations)[type._animation_name]);
    entity->_manager = _managerptr;

    // push to internal storage
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _scriptvalues[id] = _ScriptValues{MNG_TYPE_ENTITY, id, entity->_executor_id, -1, entityname, entity, nullptr};

    // enqueue if set
    if (type._force_enqueue)
        entity->enqueue();
    
    return id;
}

int Manager::spawnObject(const char *objectname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount)
        return -1;

    _ObjectType &type = _objecttypes[objectname];
    Object *object = type._allocator();
    
    // set up object
    if (type._entitytype._force_scriptsetup)
        if (_executor)
            object->scriptSetup(_executor);
    if (type._entitytype._force_entitysetup)
        if (_glenv && _animations)
            object->entitySetup(_glenv, &(*_animations)[type._entitytype._animation_name]);
    if (type._force_objectsetup)
        if (_collider)
            object->objectSetup(_collider);
    object->_manager = _managerptr;

    // push to internal storage
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(object);
    _scriptvalues[id] = _ScriptValues{MNG_TYPE_OBJECT, id, object->_executor_id, object->_collider_id, objectname, object, object};

    // enqueue if set
    if (type._entitytype._force_enqueue)
        object->enqueue();

    return id;
}

Entity *Manager::getEntity(int id) {
    if (_ids.at(id))
        return _scriptvalues[id]._entity_ref;
    else
        return nullptr;
}

Object *Manager::getObject(int id) {
    if (_ids.at(id))
        return _scriptvalues[id]._object_ref;
    else
        return nullptr;
}

void Manager::removeEntity(int id) {
    if (!_ids.at(id))
        return;

    _ScriptValues &values = _scriptvalues[id];
    _EntityType &type = _entitytypes[values._manager_name];

    // remove from anything it was pushed into
    if (type._force_scriptsetup)
        if (_executor)
            _executor->erase(values._executor_id);
    
    _ids.erase_at(values._manager_id);
};

void Manager::removeObject(int id) {
    if (!_ids.at(id))
        return;
        
    _ScriptValues &values = _scriptvalues[id];
    _ObjectType &type = _objecttypes[values._manager_name];

    // remove from anything it was pushed into
    if (type._entitytype._force_scriptsetup)
        if (_executor)
            _executor->erase(values._executor_id);
    if (type._force_objectsetup)
        if (_collider)
            _collider->erase(values._collider_id);
    
    _ids.erase_at(values._manager_id);
};

void Manager::addEntity(std::function<Entity*(void)> allocator, const char *name, bool force_scriptsetup, bool force_enqueue, bool force_entitysetup, const char *animation_name) {
    if (!hasEntity(name))
        _entitytypes[name] = _EntityType{
            force_scriptsetup,
            force_enqueue,
            force_entitysetup,
            animation_name,
            allocator
        };
}

void Manager::addObject(std::function<Object*(void)> allocator, const char *name, bool force_scriptsetup, bool force_enqueue, bool force_entitysetup, const char *animation_name, bool force_objectsetup) {
    if (!hasObject(name))
        _objecttypes[name] = _ObjectType{
            _EntityType{
                force_scriptsetup,
                force_enqueue,
                force_entitysetup,
                animation_name,
                nullptr
            },
            force_objectsetup,
            allocator
        };
}

void Manager::setExecutor(Executor *executor) { if (!_executor) _executor = executor; }
void Manager::setGLEnv(GLEnv *glenv) { if (!_glenv) _glenv = glenv; }
void Manager::setAnimations(std::unordered_map<std::string, Animation> *animations) { if (!_animations) _animations = animations; }
void Manager::setCollider(Collider *collider) { if (!_collider) _collider = collider; }

// --------------------------------------------------------------------------------------------------------------------------

ManagerPtr::ManagerPtr(Manager *manager) : _manager(manager) {}

bool ManagerPtr::hasEntity(const char *entityname) {
    return _manager->hasEntity(entityname);
}

bool ManagerPtr::hasObject(const char *objectname) {
    return _manager->hasObject(objectname);
}

int ManagerPtr::spawnEntity(const char *entityname) {
    return _manager->spawnEntity(entityname);
}

int ManagerPtr::spawnObject(const char *objectname) {
    return _manager->spawnObject(objectname);
}

Entity *ManagerPtr::getEntity(int id) {
    return _manager->getEntity(id);
}

Object *ManagerPtr::getObject(int id) {
    return _manager->getObject(id);
}