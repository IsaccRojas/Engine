#pragma once

#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"

class Collider;
class Manager;

class Object : public Entity {
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

#endif