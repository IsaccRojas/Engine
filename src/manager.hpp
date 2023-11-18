#pragma once

#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include "object.hpp"

class ManagerPtr;

/* Class to manage and control internal instances of Scripts,
   Entities and Objects, created via Spawners. Can also be given 
   an executor and collider to automatically pass instances
   to these mechanisms.
*/

enum ManagerType{ MNG_TYPE_NONE, MNG_TYPE_SCRIPT, MNG_TYPE_ENTITY, MNG_TYPE_OBJECT };

class Manager {
    // struct holding entity information mapped to a name
    struct _ScriptType {
        bool _force_scriptsetup;
        bool _force_enqueue;
        std::function<Script*(void)> _allocator = nullptr;
    };

    // struct holding entity information mapped to a name
    struct _EntityType {
        _ScriptType _scripttype;
        bool _force_entitysetup;
        std::string _animation_name;
        std::function<Entity*(void)> _allocator = nullptr;
    };

    // struct holding object information mapped to a name
    struct _ObjectType {
        _EntityType _entitytype;
        bool _force_objectsetup;
        std::function<Object*(void)> _allocator = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed entity
    struct _ScriptValues {
        ManagerType _type;
        int _manager_id;
        const char *_manager_name;
        Script *_script_ref;
        Entity *_entity_ref;
        Object *_object_ref;
    };

    std::unordered_map<std::string, _ScriptType> _scripttypes;
    std::unordered_map<std::string, _EntityType> _entitytypes;
    std::unordered_map<std::string, _ObjectType> _objecttypes;

    // "managed" structures
    Executor *_executor;
    GLEnv *_glenv;
    std::unordered_map<std::string, Animation> *_animations;
    Collider *_collider;
    ManagerPtr *_managerptr;

    // IDs to distribute to Scripts
    Partitioner _ids;
    // vector of unique_ptrs of Scripts
    std::vector<std::unique_ptr<Script>> _scripts;
    // vector of _ScriptValues
    std::vector<_ScriptValues> _scriptvalues;
    int _maxcount;
public:
    Manager(int maxcount);
    ~Manager();

    bool hasScript(const char *scriptname);
    bool hasEntity(const char *entityname);
    bool hasObject(const char *objectname);
    int spawnScript(const char *scriptname);
    int spawnEntity(const char *entityname);
    int spawnObject(const char *objectname);
    Script *getScript(int id);
    Entity *getEntity(int id);
    Object *getObject(int id);
    void remove(int id);
    void addScript(std::function<Script*(void)> allocator, const char *name, bool force_scriptsetup, bool force_enqueue);
    void addEntity(std::function<Entity*(void)> allocator, const char *name, bool force_scriptsetup, bool force_enqueue, bool force_entitysetup, const char *animation_name);
    void addObject(std::function<Object*(void)> allocator, const char *name, bool force_scriptsetup, bool force_enqueue, bool force_entitysetup, const char *animation_name, bool force_objectsetup);
    
    void setExecutor(Executor *executor);
    void setGLEnv(GLEnv *glenv);
    void setAnimations(std::unordered_map<std::string, Animation> *animations);
    void setCollider(Collider *collider);

    int getMaxID();
};

// --------------------------------------------------------------------------------------------------------------------------

/* Class to wrap Manager and only expose certain methods to managed
   units.
*/
class ManagerPtr {
    Manager *_manager;
public:
    ManagerPtr(Manager *manager);
    ~ManagerPtr();
    bool hasScript(const char *scriptname);
    bool hasEntity(const char *entityname);
    bool hasObject(const char *objectname);
    int spawnScript(const char *scriptname);
    int spawnEntity(const char *entityname);
    int spawnObject(const char *objectname);
    Script *getScript(int id);
    Entity *getEntity(int id);
    Object *getObject(int id);
    int getMaxID();
};  

#endif