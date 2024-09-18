#ifndef PHYSENV_HPP_
#define PHYSENV_HPP_

#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "filter.hpp"
#include "commonexcept.hpp"
#include <functional>
#include <list>

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
   
   // used to check ownership and for erasure
   PhysEnv *_physenv;
   std::list<Box>::iterator _this_iter;

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

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class PhysEnv {
   // Box storage
   std::list<Box> _boxes;

public:
   PhysEnv();
   ~PhysEnv();
   // default copy assignment/construction are fine

   /* Generates an active Box in system. You must call the step() method on the environment or a reference 
      to the Box itself to move it within the physical space.
      pos - GLM vec3 position of Box
      dim - GLM vec3 dimensions of Box
      vel - GLM vec3 velocity of Box
      callback - void(Box*) function pointer to callback of Box
      Returns a reference to the Box that is valid until it is erased from the environment.
   */
   Box *push(glm::vec3 pos, glm::vec3 dim, glm::vec3 vel, std::function<void(Box*)> callback);
   /* Removes the Box reference from the system. This will cause the provided reference to be invalid.
   */
   void erase(Box *box);

   /* Unsets all collided flags for all contained Boxes. */
   void unsetCollidedFlags();
   /* Detects collision between all Boxes within the system. This is done by iterating on all Boxes
      in a pair-wise fashion. All collided boxes have their collision callback invoked, and their
      collided flag set.
   */
   void detectCollision();
   /* Advances every internal Box one step in time. */
   void step();

   /* Returns true if the provided Box belongs to this PhysEnv instance. */
   bool hasBox(Box *box);

   /* Removes all boxes within this environment. Invalidates any returned references. */
   void clear();

   /* Detects and handles AABB collision between two provided Boxes. */
   static void collisionAABB(Box &box1, Box &box2);
};

/* Rotates the vector with respect to the Z axis, within a range of [-deg, deg]. */
glm::vec3 random_angle(glm::vec3 v, float deg);

#endif