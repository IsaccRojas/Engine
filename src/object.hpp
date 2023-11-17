#pragma once

#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"

class Collider;
class Manager;

class Object : public Entity {
    friend Collider;
    friend Manager;
    
    // environmental references
    Collider *_collider;    
    int _collider_id;

    bool _collider_ready;
    bool _collision_enabled;

    // called by collider
    void _collision(int x);

    void _initEntity();
    void _baseEntity();
    void _killEntity();

protected:
    // physics variables
    glm::vec3 _physpos;
    glm::vec3 _physdim;

    virtual void _initObject();
    virtual void _baseObject();
    virtual void _killObject();
    virtual void _collisionObject(int x);

public:
    Object(glm::vec3 dimensions = glm::vec3(0.0f));
    ~Object();

    /* Sets the object up with a collider. This enables the use of 
       the enableCollision() and disableCollision() methods. Does not
       push the object into the collider.
    */
    void objectSetup(Collider *collider);

    /* Pushes the object into the collider. */
    void enableCollision();

    /* Removes the object from the collider. */
    void disableCollision();

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
    int push(Object *object);
    void erase(int id);

    // checks for pair-wise collision between all pushed objects
    void collide();

    static bool detectCollision(const Object &obj1, const Object &obj2);
};

#endif