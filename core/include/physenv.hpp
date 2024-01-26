#ifndef PHYSENV_HPP_
#define PHYSENV_HPP_

#include "util.hpp"
#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "filter.hpp"
#include <functional>

/* Class to encapsulate physical Box-shaped data for use by a PhysEnv instance.
   - position - location of box in 3D space
   - dimensions - dimensions of box in 3D space
   - velocity - current velocity of box in 3D space
   - orientation - orientation of box on XY plane
   - callback - collision callback
*/
class Box {
    FilterState _filterstate;
    glm::vec3 _prevpos;
    std::function<void(Box*)> _callback;
    bool _collision_correction;
public:
    // physics variables
    glm::vec3 pos;
    glm::vec3 dim;
    glm::vec3 vel;
    /* position - location of box in 3D space
       dimensions - dimensions of box in 3D space
       velocity - current velocity of box in 3D space
       callback - collision callback
    */
    Box(glm::vec3 position, glm::vec3 dimensions, glm::vec3 velocity, std::function<void(Box*)> callback);
    Box(const Box &other);
    Box();
    Box& operator=(const Box &other);
    ~Box();

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

    /* Sets the box's collision correction flag. */
    void setCorrection(bool correction);

    /* Gets the box's collision correction flag. */
    bool getCorrection();

    /* Gets the box's previous position. */
    glm::vec3 &getPrevPos();


};

/* class PhysEnv
   Encapsulates all physics-related environment data and methods.
   Contains Boxes and can detect collision between them, invoking their stored collision callbacks.
   The maximum amount of Boxes allowed by the system can be specified. This also guarantees that 
   no more than max_count IDs will be generated and tracked. The environment will fail to generate
   new Boxes if more than the allowed amount are allocated.
*/
class PhysEnv {
    /* environment system variables */

    // IDs to distribute to Boxes
    Partitioner _ids;
    // internal Box storage
    std::vector<Box> _boxes;
    // maximum number of active Boxes allowed
    unsigned _maxcount;

public:
    /* max_count - maximum number of boxes allowed to be active */
    PhysEnv(unsigned maxcount);
    ~PhysEnv();

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
    int genBox(glm::vec3 pos, glm::vec3 dim, glm::vec3 vel, std::function<void(Box*)> callback);

    /* Returns a raw Box pointer to the Box with the specified ID. 
       i - ID of Box to get reference of
    */
    Box *get(int i);

    /* Advances every internal Box one step in time. */
    void step();

    /* Removes the Box with the provided ID from the system. This will cause the provided ID to be 
       invalid until returned again by the genBox() method. Note that this method does not actually
       free any memory; it simply makes the specific ID usable again by the system. Attempting to use
       the same ID after erasing it and before receiving it again by genBox() will result in undefined 
       behavior.
       id - ID of Box to remove
       Returns 0 on success, -1 on failure.
    */
    int remove(int id);

    /* Detects collision between all Boxes within the system. This is done by iterating on all Boxes
       in a pair-wise fashion.
    */
    void detectCollision();

    /* Returns all active IDs in system. (note that this instantiates a vector and will take O(n) time) */
    std::vector<int> getids();

    /* Detects and handles AABB collision between two provided Boxes. */
    static void collisionAABB(Box &box1, Box &box2);
};

/* Rotates the vector with respect to the Z axis, within a range of [-deg, deg]. */
glm::vec3 random_angle(glm::vec3 v, float deg);

#endif