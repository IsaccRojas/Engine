#pragma once

#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"

class Collider;
class ObjectManager;

class Object : public Entity {
    friend ObjectManager;

    // environmental references
    Collider *_collider;    
    int _collider_id;
    bool _collider_ready;

    bool _collision_enabled;

    void _initEntity();
    void _baseEntity();
    void _killEntity();

    // physics variables
    glm::vec3 _physpos;
    glm::vec3 _physvel;
    glm::vec3 _physdim;

    // Manager instance that owns this Object; maintained by Manager
    ObjectManager *_objectmanager;

protected:

    virtual void _initObject();
    virtual void _baseObject();
    virtual void _killObject();
    virtual void _collisionObject(Object *other);

public:
    Object();
    virtual ~Object();

    /* Sets the object up with a collider. This enables the use of 
       the enableCollision() and disableCollision() methods. Does not
       push the object into the collider.
    */
    void objectSetup(Collider *collider);

    /* Resets internal collider flags.
    */
    void objectResetFlags();
    
    /* Resets collider information.
    */
    void objectResetCollider();

    // called by collider
    void collide(Object *other);

    /* Pushes the object into the collider. */
    void enableCollision();

    /* Removes the object from the collider. */
    void disableCollision();

    glm::vec3 getPhysPos();
    void setPhysPos(glm::vec3 newpos);
    glm::vec3 getPhysVel();
    void setPhysVel(glm::vec3 newvel);
    glm::vec3 getPhysDim();
    void setPhysDim(glm::vec3 newdim);

    bool hasCollisionEnabled();

    ObjectManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

// used to detect collision between inserted objects
class Collider {
    std::vector<Object*> _objects;
    Partitioner _ids;

    int _maxcount;
public:
    Collider(int maxcount);
    ~Collider();
    
    int push(Object *object);
    void erase(int id);

    // checks for pair-wise collision between all pushed objects
    void collide();

    static bool detectCollision(Object &obj1, Object &obj2);
};

// --------------------------------------------------------------------------------------------------------------------------

/* Class to manage and control internal instances of Objects. Can also be given an 
   executor, glenv and allocator instance to automatically pass instances to these 
   mechanisms.
*/
class ObjectManager : public EntityManager {
public:
    // struct holding object information mapped to a name
    struct ObjectType {
        bool _force_objectsetup;
        std::function<Object*(void)> _allocator = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed object
    struct ObjectValues {
        Object *_object_ref;
    };

protected:
    // internal variables for added objects and existing objects
    std::unordered_map<std::string, ObjectType> _objecttypes;
    std::vector<ObjectValues> _objectvalues;

    Collider *_collider;

    void _objectSetup(Object *object, ObjectType &objecttype, EntityType &entitytype, ScriptType &scripttype, int id);
    void _objectRemoval(ObjectValues &objectvalues, EntityValues &entityvalues, ScriptValues &scriptvalues);

public:
    ObjectManager(int maxcount);
    ~ObjectManager();
    
    bool hasObject(const char *objectname);
    Object *getObject(int id);
    int spawnScript(const char *scriptname);
    int spawnEntity(const char *entityname);
    int spawnObject(const char *objectname);
    void addObject(std::function<Object*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name, bool force_objectsetup);
    void remove(int id);

    void setCollider(Collider *collider);
};

#endif