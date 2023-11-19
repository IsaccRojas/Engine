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
        _scriptvalues.push_back(_ScriptValues{MNG_TYPE_NONE, -1, NULL, nullptr, nullptr, nullptr});
    }
    _managerptr = new ManagerPtr(this);
}
Manager::~Manager() {
    /* TODO: consider whether it is necessary to do this or not
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
    */

    delete _managerptr;
}

bool Manager::hasScript(const char *scriptname) { return !(_scripttypes.find(scriptname) == _scripttypes.end()); }

bool Manager::hasEntity(const char *entityname) { return !(_entitytypes.find(entityname) == _entitytypes.end()); }

bool Manager::hasObject(const char *objectname) { return !(_objecttypes.find(objectname) == _objecttypes.end()); }

int Manager::spawnScript(const char *scriptname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Manager " << this << std::endl;
        return -1;
    }

    _ScriptType &type = _scripttypes[scriptname];
    Script *script = type._allocator();
    
    // set up script
    if (type._force_scriptsetup)
        if (_executor)
            script->scriptSetup(_executor);
    script->_manager = _managerptr;
    script->setType(type._internal_type);

    // push to internal storage
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(script);
    _scriptvalues[id] = _ScriptValues{MNG_TYPE_SCRIPT, id, scriptname, script, nullptr, nullptr};

    // enqueue if set
    if (type._force_enqueue)
        script->enqueue();
    
    return id;
}

int Manager::spawnEntity(const char *entityname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Manager " << this << std::endl;
        return -1;
    }

    _EntityType &type = _entitytypes[entityname];
    Entity *entity = type._allocator();
    
    // set up entity
    if (type._scripttype._force_scriptsetup)
        if (_executor)
            entity->scriptSetup(_executor);
    if (type._force_entitysetup)
        if (_glenv && _animations)
            entity->entitySetup(_glenv, &(*_animations)[type._animation_name]);
    entity->_manager = _managerptr;
    entity->setType(type._scripttype._internal_type);

    // push to internal storage
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(entity);
    _scriptvalues[id] = _ScriptValues{MNG_TYPE_ENTITY, id, entityname, entity, entity, nullptr};

    // enqueue if set
    if (type._scripttype._force_enqueue)
        entity->enqueue();
    
    return id;
}

int Manager::spawnObject(const char *objectname) {
    // fail if exceeding max size
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Manager " << this << std::endl;
        return -1;
    }

    _ObjectType &type = _objecttypes[objectname];
    Object *object = type._allocator();
    
    // set up object
    if (type._entitytype._scripttype._force_scriptsetup)
        if (_executor)
            object->scriptSetup(_executor);
    if (type._entitytype._force_entitysetup)
        if (_glenv && _animations)
            object->entitySetup(_glenv, &(*_animations)[type._entitytype._animation_name]);
    if (type._force_objectsetup)
        if (_collider)
            object->objectSetup(_collider);
    object->_manager = _managerptr;
    object->setType(type._entitytype._scripttype._internal_type);

    // push to internal storage
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(object);
    _scriptvalues[id] = _ScriptValues{MNG_TYPE_OBJECT, id, objectname, object, object, object};

    // enqueue if set
    if (type._entitytype._scripttype._force_enqueue)
        object->enqueue();

    return id;
}

Script *Manager::getScript(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._script_ref;
    else
        return nullptr;
}

Entity *Manager::getEntity(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._entity_ref;
    else
        return nullptr;
}

Object *Manager::getObject(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._object_ref;
    else
        return nullptr;
}

std::string Manager::getName(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._manager_name;
    return "";
}

void Manager::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Manager " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Manager " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        // get info and remove from respective systems
        _ScriptValues &values = _scriptvalues[id];
        switch (values._manager_type) {
            case MNG_TYPE_OBJECT:
                values._object_ref->objectResetCollider();
                values._object_ref->eraseQuad();
                values._object_ref->scriptResetExec();
                break;
            case MNG_TYPE_ENTITY:
                values._entity_ref->eraseQuad();
                values._entity_ref->scriptResetExec();
                break;
            case MNG_TYPE_SCRIPT:
                values._script_ref->scriptResetExec();
                break;
            default:
                std::cerr << "WARN: unknown type removed from Manager " << this << std::endl;
                break;
        }
    
        _ids.erase_at(_scriptvalues[id]._manager_id);
    }
}

void Manager::addScript(std::function<Script*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill) {
    if (!hasScript(name))
        _scripttypes[name] = _ScriptType{
            type,
            force_scriptsetup,
            force_enqueue,
            force_removeonkill,
            allocator
        };
}

void Manager::addEntity(std::function<Entity*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name) {
    if (!hasEntity(name))
        _entitytypes[name] = _EntityType{
            _ScriptType{
                type,
                force_scriptsetup,
                force_enqueue,
                force_removeonkill,
                nullptr
            },
            force_entitysetup,
            animation_name,
            allocator
        };
}

void Manager::addObject(std::function<Object*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name, bool force_objectsetup) {
    if (!hasObject(name))
        _objecttypes[name] = _ObjectType{
            _EntityType{
                _ScriptType{
                    type,
                    force_scriptsetup,
                    force_enqueue,
                    force_removeonkill,
                    nullptr
                },
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

int Manager::getMaxID() { return _ids.size(); }

// --------------------------------------------------------------------------------------------------------------------------

ManagerPtr::ManagerPtr(Manager *manager) : _manager(manager) {}
ManagerPtr::~ManagerPtr() {}

bool ManagerPtr::hasScript(const char *scriptname) {
    return _manager->hasScript(scriptname);
}

bool ManagerPtr::hasEntity(const char *entityname) {
    return _manager->hasEntity(entityname);
}

bool ManagerPtr::hasObject(const char *objectname) {
    return _manager->hasObject(objectname);
}

int ManagerPtr::spawnScript(const char *scriptname) {
    return _manager->spawnScript(scriptname);
}

int ManagerPtr::spawnEntity(const char *entityname) {
    return _manager->spawnEntity(entityname);
}

int ManagerPtr::spawnObject(const char *objectname) {
    return _manager->spawnObject(objectname);
}

Script *ManagerPtr::getScript(int id) {
    return _manager->getScript(id);
}

Entity *ManagerPtr::getEntity(int id) {
    return _manager->getEntity(id);
}

Object *ManagerPtr::getObject(int id) {
    return _manager->getObject(id);
}

int ManagerPtr::getMaxID() {
    return _manager->getMaxID();
}

void ManagerPtr::remove(int id) {
    return _manager->remove(id);
}