#ifndef PHYSSPACE_HPP_
#define PHYSSPACE_HPP_

#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "commonexcept.hpp"
#include "filter.hpp"
#include <functional>
#include <list>

struct Transform {
   glm::vec3 pos = glm::vec3(0.0f);
   glm::vec3 scale = glm::vec3(0.0f);
   // default copy assignment/construction are fine
};

template <class T>
class PhysSpace;

/* abstract class ColliderInterface
   Used to implement specialized collision detection algorithms as well as supplementary
   data to physical objects; used by PhysSpace<T> instances to allocate and manage subtype
   instances. Template parameter must be the same type T as the implementing class.
*/
template <class T>
class ColliderInterface {
    friend PhysSpace<T>;
    
    // used to check ownership and for erasure
    PhysSpace<T> *_physspace;
    typename std::list<T*>::iterator _this_iter;

    unsigned _collided_count;

    FilterState _filterstate;
    glm::vec3 _prev_pos;
    std::function<void(T*)> _callback;

    bool _collision_enabled;

protected:
    ColliderInterface() :
        _physspace(nullptr),
        _collided_count(false),
        _prev_pos(glm::vec3(0.0f)),
        _callback(nullptr),
        _collision_enabled(true),
        transform(Transform{glm::vec3(0.0f), glm::vec3(0.0f)}),
        vel(glm::vec3(0.0f)),
        mass(1.0f)
    {}
    ColliderInterface(ColliderInterface<T> &&other) { operator=(std::move(other)); }
    ColliderInterface(const ColliderInterface<T> &other) = delete;

    ColliderInterface<T> &operator=(ColliderInterface<T> &&other) {
        if (this != &other) {
            _physspace = other._physspace;
            _this_iter = other._this_iter;
            _collided_count = other._collided_count;
            _filterstate = other._filterstate;
            _prev_pos = other._prev_pos;
            _callback = other._callback;
            _collision_enabled = other._collision_enabled;
            transform = other.transform;
            vel = other.vel;
            mass = other.mass;
            other._physspace = nullptr;
            other._collided_count = 0;
            other._filterstate.setFilter(nullptr);
            other._prev_pos = glm::vec3(0.0f);
            other._callback = nullptr;
            other._collision_enabled = true;
        }
        return *this;
    }

public:
    // physics variables
    Transform transform;
    glm::vec3 vel;
    float mass;

    virtual ~ColliderInterface() {
        if (_physspace)
            _physspace->erase(this);
    }

    ColliderInterface<T> &operator=(const ColliderInterface<T> &other) = delete;

    /* Sets the instance's collision callback. */
    void setCallback(std::function<void(T*)> callback) {
        _callback = callback;
    }

    /* Enables or disables collision for this collider. */
    void enableCollision(bool state) { _collision_enabled = state; }

    /* Applies the current velocity to the position, and internally stores the previous position. */
    void step() {
        _prev_pos = transform.pos;
        transform.pos = transform.pos + vel;
    }

    /* Invokes internal callback. */
    void collide(T *t) {
        if (_callback)
            _callback(t);
    }

    /* Gets the instance's collision filter state. */
    FilterState &filterstate() {
        return _filterstate;
    }

    /* Gets the instance's previous position. */
    glm::vec3 &prevpos() {
        return _prev_pos;
    }

    /* Returns whether the instance was collided with or not. This is only set and unset by an owning PhysSpace. */
    unsigned getCollidedCount() { return _collided_count; }

    /* Computes and handles collision between this and the provided instance. */
    virtual bool computeCollision(T *other) = 0;
};

// --------------------------------------------------------------------------------------------------------------------------

/* class PhysSpace
   Encapsulates physics-related data and methods corresponding to implementations of the ColliderInterface
   class. Contains instances of the implementing class and can detect collision between them, invoking their
   stored collision callbacks.
*/
template <class T>
class PhysSpace {
    // Collider storage
    std::list<T*> _Ts;

public:
    PhysSpace() {}
    PhysSpace(PhysSpace<T> &&other) { operator=(std::move(other._Ts.clear())); }
    PhysSpace(const PhysSpace<T> &other) = delete;
    ~PhysSpace() {
        for (auto &t : _Ts) {
            t->_physspace = nullptr;
            delete t;
        }
    }

    PhysSpace<T> &operator=(PhysSpace<T> &&other) {
        if (this != &other) {
            _Ts = other._Ts;
            other._Ts.clear();
        }
        return *this;
    }
    PhysSpace<T> &operator=(const PhysSpace<T> &other) = delete;

    /* Generates an active instance of T in system. You must call the step() method on this or a reference 
       to the instance of T itself to move it within the physical space.
       transf - Transform of T
       vel - GLM vec3 velocity of T
       callback - void(T*) function pointer to callback of T
       Returns a reference to the instance of T that is valid until it is erased from the environment.
    */
    T *push(Transform transf, glm::vec3 vel, std::function<void(T*)> callback) {
        _Ts.push_back(new T);
        auto iter = _Ts.end();
        iter--;

        // assign iterator position and this reference to instance
        T *t = *iter;
        t->_physspace = this;
        t->_this_iter = iter;

        // initialize fields
        t->transform = transf;
        t->vel = vel;
        t->_callback = callback;

        return t;
    }

    /* Removes the reference from the system. This will cause the provided reference to be invalid. */
    void erase(ColliderInterface<T> *t) {
        if (t->_physspace != this)
            throw std::runtime_error("Attempt to erase Box from PhysEnv that does not own it");
        
        _Ts.erase(t->_this_iter);

        t->_physspace = nullptr;
        delete t;
    }

    /* Sets collided count to 0 for all contained instances. */
    void resetCollidedCount() {
        for (auto &t : _Ts)
            t->_collided_count = 0;
    }
    
    /* Detects collision between all instances within the system. This is done by iterating on all elements
       in a pair-wise fashion. All collided instances have their collision callback invoked, and their
       collided count incremented.
    */
    void detectCollision() {
        // perform pair-wise collision detection
        for (auto iter1 = _Ts.begin(); iter1 != _Ts.end(); iter1++) {

            // get T and skip if scale is zeroed out
            T *t1 = *iter1;
            if (!(t1->_collision_enabled) || (t1->transform.scale == glm::vec3(0.0f)))
                continue;

            auto iter2 = iter1;
            iter2++;
            for (;iter2 != _Ts.end(); iter2++) {

                // get other T and skip if scale is zeroed out (check t1's enable flag again in case it was unset this outer loop iteration)
                T *t2 = *iter2;
                if (!(t1->_collision_enabled) || !(t2->_collision_enabled) || (t2->transform.scale == glm::vec3(0.0f)))
                    continue;

                // test filters against each other's IDs
                bool f1 = t1->filterstate().hasFilter();
                bool f2 = t2->filterstate().hasFilter();
                
                // if both have a filter, collide if both pass
                // if neither have a filter, collide
                // if only one has a filter, skip
                if (f1 != f2)
                    continue;
                if (
                    (!f1 && !f2) ||
                        (t1->filterstate().pass(t2->filterstate().id()) &&
                        t2->filterstate().pass(t1->filterstate().id()))
                ) {
                    // detect and handle collision
                    if (t1->computeCollision(t2)) {
                        t1->_collided_count++;
                        t2->_collided_count++;

                        // run collision handlers
                        t1->collide(t2);
                        t2->collide(t1);
                    }
                }

            }

        }
    }

    /* Advances every internal instance one step in time. */
    void step() {
        for (auto &t : _Ts)
            t->step();
    }

    /* Returns true if the provided instance belongs to this PhysSpace instance. */
    bool hasInstance(T *t) {
        return (t->_physspace == this);
    };

    /* Removes all instances within this environment. Invalidates any existing references. */
    void clear() {
        _Ts.clear();
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* Representation of an axis-aligned box in physical space. */
class Box : public ColliderInterface<Box> {
public:
    Box() {}
    Box(Box &&other) { operator=(std::move(other)); }
    Box(const Box &other) = delete;
    ~Box() { /* automatic destruction is fine */ }

    Box &operator=(Box &&other) {
        if (this != &other) {
            ColliderInterface<Box>::operator=(std::move(other));
        }
        return *this;
    }
    Box &operator=(const Box &other) = delete;

    // default move assignment/construction are fine

    /* Performs AABB collision detection. */
    bool computeCollision(Box *other) override {
        glm::vec3 &pos1 = transform.pos;
        glm::vec3 &dim1 = transform.scale;
        glm::vec3 &pos2 = other->transform.pos;
        glm::vec3 &dim2 = other->transform.scale;

        // get current collision
        float coll_hor_space = glm::abs(pos1.x - pos2.x) - ((dim1.x + dim2.x) / 2.0f);
        float coll_ver_space = glm::abs(pos1.y - pos2.y) - ((dim1.y + dim2.y) / 2.0f);
        float coll_dep_space = glm::abs(pos1.z - pos2.z) - ((dim1.z + dim2.z) / 2.0f);

        if (coll_hor_space < 0.0f && coll_ver_space < 0.0f && coll_dep_space < 0.0f)
            return true;
        return false;
    }
};

/* Representation of a sphere in physical space. */
class Sphere : public ColliderInterface<Sphere> {
public:
    Sphere() : radius(0.0f) {}
    Sphere(Sphere &&other) { operator=(std::move(other)); }
    Sphere(const Sphere &other) = delete;
    ~Sphere() { /* automatic destruction is fine */ }

    Sphere &operator=(Sphere &&other) {
        if (this != &other) {
            ColliderInterface<Sphere>::operator=(std::move(other));
            radius = other.radius;
            other.radius = 0.0f;
        }
        return *this;
    }
    Sphere &operator=(const Sphere &other) = delete;

    float radius;

    /* Ignores scale. */
    bool computeCollision(Sphere *other) override {
        return ((glm::distance(transform.pos, other->transform.pos) - (radius + other->radius)) < 0.0f);
    }
};

// --------------------------------------------------------------------------------------------------------------------------

/* Rotates the vector with respect to the Z axis, within a range of [-deg, deg]. */
glm::vec3 random_angle(glm::vec3 v, float deg);

#endif