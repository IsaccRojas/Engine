#ifndef PHYSICS_HPP_
#define PHYSICS_HPP_

#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "commonexcept.hpp"
#include "filter.hpp"
#include <functional>
#include "managedlist.hpp"

typedef std::unordered_map<std::string, Filter> unordered_map_string_Filter_t;

struct Transform {
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(0.0f);
    // default copy assignment/construction are fine
};

// prototype
class CollisionSpace;

/* class Collider
   Represents a physical presence capable of collision within a CollisionSpace.
*/
class Collider {
    friend CollisionSpace;

    // fields maintained by owning CollisionSpace
    CollisionSpace *_collisionspace;
    std::list<Collider>::iterator _this_iter;
    FilterState _filterstate;

public:
    Collider(Collider &&other);
    Collider();
    Collider(const Collider&) = delete;
    virtual ~Collider();

    Collider& operator=(Collider &&other);
    Collider& operator=(const Collider&) = delete;

    // physics variables
    Transform transform;
    glm::vec3 vel;
    float mass;
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ColliderView
   Contains a Collider reference and wraps access to Collider data without owning it. Invalid if the viewed Collider is destroyed.
*/
class ColliderView {
    friend CollisionSpace;
    Collider *_collider;
public:
    ColliderView(Collider *collider);
};

// --------------------------------------------------------------------------------------------------------------------------

/* class CollisionSpace
   Encapsulates a physical space for contained Colliders to interact.
*/
class CollisionSpace {
    // reference to map of filters
    unordered_map_string_Filter_t *_filters;

    // flag to store if instance was initialized or not
    bool _initialized;

    /* Script data structures */
    // memory-managed list of Collider references
    ManagedList<Collider> _boxes;

    bool _initialized;

protected:
    // initializes Collider's CollisionSpace-related fields
    void _setupCollider(Collider *collider, const char *filter);

    // erases the passed Collider; it is undefined behavior to use the ColliderView after this call
    void _erase(Collider *collider);

public:
    /* Calls init() with the provided arguments. */
    CollisionSpace(unordered_map_string_Filter_t *filters);
    CollisionSpace();
    CollisionSpace(CollisionSpace &&other);
    CollisionSpace(const CollisionSpace &other) = delete;
    virtual ~CollisionSpace();

    CollisionSpace &operator=(CollisionSpace &&other);
    CollisionSpace &operator=(const CollisionSpace &other) = delete;

    /* Initializes internal CollisionSpace data. It is undefined behavior to make calls on this instance
        before calling this and after uninit().
    */
    void init(unordered_map_string_Filter_t *filters);
    void uninit();

    /* Spawns a Collider and returns a ColliderView. */
    ColliderView spawnCollider(Transform transform, glm::vec3 vel, const char *filter_name);

    /* Erases the Collider referenced by the provided ColliderView. */
    void erase(ColliderView colliderview);

    /* Sets collided count to 0 for all contained instances. */
    void resetCollidedCount();

    /* Detects collision between all instances within the system. This is done by iterating on all elements
        in a pair-wise fashion. All collided instances have their collision callback invoked, and their
        collided count incremented.
    */
    void detectCollision();

    /* Advances every internal instance one step in time. */
    void step();

    /* Returns the number of Colliders in this CollisionSpace. */
    unsigned getCount();

    /* Returns whether or not this CollisionSpace instance has been initialized or not. */
    bool initialized();
};

#endif