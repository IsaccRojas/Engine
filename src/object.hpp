#pragma once

#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"
#include "glm\gtx\rotate_vector.hpp"

class Collider;
class ObjectManager;

class Object : public Entity {
    friend ObjectManager;

    // environmental references
    Collider *_collider;    
    int _collider_id;
    bool _collider_ready;
    glm::vec3 _collision_prevpos;

    bool _collision_enabled;
    bool _collision_correction;
    bool _collision_elastic;

    void _initEntity();
    void _baseEntity();
    void _killEntity();

    // physics variables
    glm::vec3 _physpos;
    glm::vec3 _physvel;
    glm::vec3 _physdim;
    float _physorient;

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
    
    /* Resets collider information.
    */
    void objectResetCollider();

    // called by collider
    void collide(Object *other);

    /* Pushes the object into the collider. */
    void enableCollision();

    /* Removes the object from the collider. */
    void disableCollision();

    bool hasCollisionEnabled();

    /* Used to control whether position correction after collision should occur. 
       Does nothing if collision is disabled.
    */
    bool getCorrection();
    void setCorrection(bool correction);
    
    /* Used to control whether elastic velocity change after collision should occur.
       Does nothing if collision is disabled.
    */
    bool getElastic();
    void setElastic(bool elastic);

    glm::vec3 getPhysPos();
    void setPhysPos(glm::vec3 newpos);
    glm::vec3 getPhysVel();
    void setPhysVel(glm::vec3 newvel);
    glm::vec3 getPhysDim();
    void setPhysDim(glm::vec3 newdim);
    float getPhysOrient();
    void setPhysOrient(float neworient);
    glm::vec3 getPrevPos();

    ObjectManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

// used to detect collision between inserted objects
class Collider {
    std::vector<Object*> _objects;
    Partitioner _ids;

    unsigned _maxcount;
public:
    Collider(unsigned maxcount);
    ~Collider();
    
    int push(Object *object);
    void erase(int id);

    // checks for pair-wise collision between all pushed objects
    void collide();
    
    /* Detects and handles AABB collision.
    */
    static void collisionAABB(Object *obj1, Object *obj2);

    /* Gets distance of collision. This is equal to the distance between the objects' centers
       minus the overlap of that distance over each figure. If this value is less than 0, there
       is a collision.
    */
    static float collisionDistance(Object &obj1, Object &obj2);
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

/* Rotates the vector with respect to the Z axis, within a range of [-deg, deg]. */
glm::vec3 random_angle(glm::vec3 v, float deg);

/* Returns true if the signs of x and y are equal, false otherwise. */
bool signequal(float x, float y);

/* Returns an integer based on the "side" the vector is closest to (i.e. quadrant, but rotated 45 degrees).
   v - vector specifying direction
   dim - dimensions of quad
   orientation - orientation of quad in radians (0 is up direction, goes clockwise)

   Returns:
   0: upper side
   1: right side
   2: lower side
   3: left side
   Currently ignores 3rd axis.
*/
int side(glm::vec3 v, glm::vec3 dim, float orientation);

#endif