#pragma once

#ifndef MANAGER_HPP_
#define MANAGER_HPP_

#include "object.hpp"

/* Class to manage and control internal instances of Scripts,
   Entities and Objects, created via Spawners. Can also be given 
   an executor and collider to automatically pass instances
   to these mechanisms.
*/
class Manager {
    // references to all "managed" structures
    EntitySpawner *_entityspawner;
    ObjectSpawner *_objectspawner;
    Executor *_executor;
    Collider *_collider;
    bool _hasentityspawner;
    bool _hasobjectspawner;
    bool _hasexecutor;
    bool _hascollider;

public:

    /*
    bool hasEntity(const char *entityname);
    bool hasObject(const char *objectname);
    Entity *spawnEntity(const char *entityname);
    Object *spawnObject(const char *objectname);

    void setEntitySpawner(EntitySpawner *entityspawner);
    void setObjectSpawner(ObjectSpawner *objectspawner);
    void setExecutor(Executor *executor);
    void setCollider(Collider *collider);

    void runCollision();
    void runExec();
    void runErase();
    void kill();

    */
};

/* Class to wrap Manager and only expose certain methods to managed
   units.
*/
class ManagerPtr {
    Manager *_manager;
public:
    ManagerPtr(Manager *manager);
    bool hasEntity(const char *entityname);
    bool hasObject(const char *objectname);
    Entity *spawnEntity(const char *entityname);
    Object *spawnObject(const char *objectname);
};  

#endif