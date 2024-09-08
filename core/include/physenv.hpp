#ifndef PHYSENV_HPP_
#define PHYSENV_HPP_

#include "util.hpp"
#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "filter.hpp"
#include "commonexcept.hpp"
#include <functional>

class PhysEnv;

/* class Box
   Encapsulates physical Box-shaped data for use by a PhysEnv instance.
   - position - location of box in 3D space
   - dimensions - dimensions of box in 3D space
   - velocity - current velocity of box in 3D space
   - orientation - orientation of box on XY plane
   - callback - collision callback
*/
class Box {
    friend PhysEnv;
    bool _collided;

    FilterState _filter_state;
    glm::vec3 _prev_pos;
    std::function<void(Box*)> _callback;
public:
    // physics variables
    glm::vec3 pos;
    glm::vec3 dim;
    glm::vec3 vel;
    float mass;
    /* position - location of box in 3D space
       dimensions - dimensions of box in 3D space
       velocity - current velocity of box in 3D space
       callback - collision callback
    */
    Box(glm::vec3 position, glm::vec3 dimensions, glm::vec3 velocity, std::function<void(Box*)> callback);
    Box();
    ~Box();

    // default copy assignment/construction are fine

    /* Sets the box's collision callback. */
    void setCallback(std::function<void(Box*)> callback);

    /* Applies the current velocity to the position, and internally stores the previous position. */
    void step();

    /* Invokes internal callback. */
    void collide(Box *box);
    
    /* Sets the box's collision filter. */
    void setFilter(Filter *filter);

    /* Gets the box's collision filter state. */
    FilterState &getFilterState();

    /* Gets the box's previous position. */
    glm::vec3 &getPrevPos();

    /* Returns whether the box was collided with or not. This is only set and unset by an owning PhysEnv. */
    bool getCollided();
};

/* class PhysEnv
   Encapsulates all physics-related environment data and methods.
   Contains Boxes and can detect collision between them, invoking their stored collision callbacks.
   The maximum amount of Boxes allowed by the system can be specified. This also guarantees that 
   no more than max_count IDs will be generated and tracked. The environment will fail to generate
   new Boxes if more than the allowed amount are allocated.

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class PhysEnv {
    // IDs to distribute to Boxes
    SlotVec _ids;

    // internal Box storage
    std::vector<Box> _boxes;
    
    // maximum number of active Boxes allowed
    unsigned _max_count;
    unsigned _count;

    bool _initialized;
public:
    /* Calls init() with the provided arguments. */
    PhysEnv(unsigned max_count);
    PhysEnv();
    ~PhysEnv();

    // default copy assignment/construction are fine

    void init(unsigned max_count);
    void uninit();

    /* Generates an active Box in system. You must call the step() method on the environment or a reference 
       to the Box itself.
       pos - GLM vec3 position of Box
       dim - GLM vec3 dimensions of Box
       vel - GLM vec3 velocity of Box
       callback - void(Box*) function pointer to callback of Box
       Returns the integer ID of Box. This number can be used to index into the internal Box container and
       obtain a reference (see the get() method). This ID is unique and will be valid for the lifetime 
       of the Box (see the erase() method). If the maximum number of active Boxes allowed is exceeded, -1 is
       returned instead.
    */
    unsigned genBox(glm::vec3 pos, glm::vec3 dim, glm::vec3 vel, std::function<void(Box*)> callback);
    /* Removes the Box with the provided ID from the system. This will cause the provided ID to be 
       invalid until returned again by the genBox() method. Note that this method does not actually
       free any memory; it simply makes the specific ID usable again by the system. Attempting to use
       the same ID after erasing it and before receiving it again by genBox() will result in undefined 
       behavior.
       id - ID of Box to remove
    */
    void remove(unsigned id);

    /* Unsets all collided flags for all contained Boxes. */
    void unsetCollidedFlags();
    /* Detects collision between all Boxes within the system. This is done by iterating on all Boxes
       in a pair-wise fashion. All collided boxes have their collision callback invoked, and their
       collided flag set.
    */
    void detectCollision();
    /* Advances every internal Box one step in time. */
    void step();

    /* Returns true if the provided ID is active. */
    bool hasID(unsigned id);
    /* Returns a raw Box pointer to the Box with the specified ID. 
       id - ID of Box to get reference of
    */
    Box *getBox(unsigned id);
    /* Returns all active IDs in system. (note that this instantiates a vector and will take O(n) time) */
    std::vector<unsigned> getIDs();
    /* Returns whether this instance has been initialized or not. */
    bool getInitialized();

    /* Detects and handles AABB collision between two provided Boxes. */
    static void collisionAABB(Box &box1, Box &box2);
};

/* Rotates the vector with respect to the Z axis, within a range of [-deg, deg]. */
glm::vec3 random_angle(glm::vec3 v, float deg);

#endif