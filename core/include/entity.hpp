#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "filter.hpp"
#include "C:\dev\include\glm\glm.hpp"
#include "C:\dev\include\glm\gtx\rotate_vector.hpp"

class EntityExecutor;
class EntityCollider;
class CollisionSpace;
class EntityAllocatorInterface;

/* class EntityInterface
   Represents a ScriptInterface that belongs to an Entity.
*/
class EntityInterface : public ScriptInterface {
   friend EntityExecutor;

   EntityInterface* _parent;
   std::string _entity_name;

   EntityExecutor* _entityexecutor;
   std::list<EntityInterface*>::iterator _this_iter;
   std::string _group;

   std::vector<EntityInterface*> _entities;
   std::vector<unsigned> _quad_ids;
   std::vector<Quad*> _quads;
   std::vector<EntityCollider*> _entitycolliders;

   bool _kill_started;

   // reference to parents' transforms, or otherwise a heap-allocated transform
   Transform* _globaltransform;
   Transform _prevglobaltransform;

   std::unordered_set<EntityInterface**> _nullablerefs;

   // called by execution environment
   void _init() override;
   void _exec() override;
   void _kill() override;

protected:
   /* Functions to be overridden by children.
      - _init() is called by runInit(). runInit() is called on spawn.
      - _exec() is called by runExec(). runExec() is called on execution, each time the entity is queued.
      - _kill() is called by runKill(). runKill() is called on erasure.
      - _collide() is called by EntityCollider associated with this instance.
   */
   virtual void _initEntity();
   virtual void _execEntity();
   virtual void _killEntity();
   virtual void _collide(EntityInterface* entity);

   EntityInterface(EntityInterface&& other);
   EntityInterface(EntityInterface* parent = nullptr);

public:
   EntityInterface(const EntityInterface& other) = delete;
   virtual ~EntityInterface();

   EntityInterface& operator=(EntityInterface&& other);
   EntityInterface& operator=(const EntityInterface& other) = delete;

   void receive(EntityInterface* entity, std::string message);
   void collide(EntityInterface* entity);

   /* Inserts reference to pointer to EntityInterface that is nulled on invocation of this instance's kill method. */
   void attachNullableRef(EntityInterface** ref);
   /* Removes reference to pointer to EntityInterface. */
   void detachNullableRef(EntityInterface** ref);

   /* Starts kill on this entity and all contained entities via EntityExecutor checks. */
   void startKill();

   /* Starts kill on this entity and all contained entities, and parents, via EntityExecutor checks. */
   void startKillAll();
   
   /* Entries may be nulled by removal of individual instances from owners. */
   std::vector<EntityInterface*>& entities();
   std::vector<Quad*>& quads();
   std::vector<EntityCollider*>& entitycolliders();

   /* Current global transform. */
   Transform& globaltransform();
   /* Global transform at the time of the last invocation of update() on an owning EntityExecutor. */
   Transform getPrevGlobalTransform();

   const char* getName();
   const char* getGroup();
   bool getKillStarted();
};

// --------------------------------------------------------------------------------------------------------------------------

// struct holding EntityInterface information mapped to a name
/*
   group - named group for this entity to be inserted into; created if it does not already exist
   entity_names - names of EntityInterfaces to use for owned entity instantiation, in order (always calls runInit() method after all have been instantiated)
   quad_names - names of QuadInfos to use for Quad instantiation, in order
   entitycollider_names - names of ColliderInfos to use for Collider instantiation, in order
   spawn_callback - callback to call when spawning an instance
*/

struct EntityInfo {
   EntityAllocatorInterface* allocator;
   int preferred_queue;
   bool auto_enqueue;
   std::list<std::string> entity_names;
   std::list<std::string> quad_names;
   std::list<std::string> entitycollider_names;
   std::function<void(ScriptInterface*)> spawn_callback;
   std::function<void(ScriptInterface*)> remove_callback;
   // default copy assignment/construction are fine
};

class EntityExecutor : public ScriptExecutor {
   std::unordered_map<unsigned, EntityInterface*> _entities_id;

   // storage of entity info, mapped to names
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   // references to entities (does not own)
   std::unordered_set<EntityInterface*> _entity_refs;

   GLEnv* _glenv;
   CollisionSpace* _collisionspace;

   // can only be called from checkEntities() if entity is killed
   void _removeEntity(EntityInterface* entity);

   // spawn entity with specified parent. transform parameter is ignored if parent is not nullptr
   EntityInterface* _spawnEntityNode(const char* entity_name, EntityInterface* parent, Transform transform);
protected:
   // class to store enqueues and polymorphically spawn later
   class EntityEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor* _entityexecutor;
      Transform _transform;
   protected:
      // invokes the containing EntityExecutor's _spawnEntity() method and returns the spawned instance's reference
      virtual ScriptInterface* spawn() override;
      EntityEnqueue(EntityExecutor* entityexecutor, std::string name, Transform transform);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityExecutor)
   };
    
public:
   /* Calls init() with the provided arguments. */
   EntityExecutor(unsigned queues, GLEnv* glenv, CollisionSpace* collisionspace);
   EntityExecutor(EntityExecutor&& other);
   EntityExecutor(const EntityExecutor& other) = delete;
   virtual ~EntityExecutor() override;

   EntityExecutor& operator=(EntityExecutor&& other);
   EntityExecutor& operator=(const EntityExecutor& other) = delete;

   /* Adds an EntityInterface allocator with initialization information to this executor, allowing its given
      name to be used for future spawns.
      - info - instance of EntityInfo with allocation/initialization information
      - name - name to associate with the EntityInfo instance
   */
   void addEntity(EntityInfo info, const char* name);

   /* Spawns an entity using a name previously added to this executor, and returns its ID. */
   EntityInterface* spawnEntity(const char* entity_name, Transform transform);

   /* Enqueues an Entity to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntity(const char* entity_name, Transform transform);

   void update();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class EntityAllocatorInterface
   Is used to invoke _allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityExecutor instance.

   Stores a reference that can be checked against for existence by subtypes.
*/
class EntityAllocatorInterface : public ScriptAllocatorInterface {
   friend EntityExecutor;

protected:
   // must return a heap-allocated instance of a covariant type of EntityInterface
   virtual EntityInterface* _allocate() = 0;

   // called on deallocation; should not deallocate anything
   virtual void _onDeallocation(ScriptInterface* script) = 0;

   EntityAllocatorInterface();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityCollider
   Represents a physical presence capable of collision within a CollisionSpace.
*/
class EntityCollider {
   friend CollisionSpace;

   // fields maintained by owning CollisionSpace
   CollisionSpace* _collisionspace;
   std::list<EntityCollider*>::iterator _this_iter;
   FilterState _filterstate;

   // physics variables
   bool _collision_enabled;
   bool _transformation_is_reset;
   glm::vec3 _base_pos;
   glm::vec3 _base_scale;
   glm::vec3 _cur_pos;
   glm::vec3 _cur_scale;
   glm::vec3 _prev_pos;
   glm::vec3 _prev_scale;

   EntityInterface* _entity;

   void _storePrevTransformation();

   EntityCollider(EntityCollider&& other);
   EntityCollider();

public:
   EntityCollider(const EntityCollider&) = delete;
   virtual ~EntityCollider();

   EntityCollider& operator=(EntityCollider&& other);
   EntityCollider& operator=(const EntityCollider&) = delete;

   /* Resets position and scale to base values. */
   void resetTransformation();

   /* Applies provided Transform to pos and scale. */
   void applyTransform(Transform transform);

   /* Get base position and scale as a Transform */
   Transform getBaseTransformation();

   /* Get current position and scale as a Transform */
   Transform getCurrentTransformation();

   /* Get previous transformation of position and scale as a Transform */
   Transform getPrevTransformation();

   FilterState& filterstate();
   bool& collision_enabled();
};

// --------------------------------------------------------------------------------------------------------------------------

struct EntityColliderInfo {
   glm::vec3 pos;
   glm::vec3 scale;
   Filter filter;
};

/* class CollisionSpace
   Encapsulates a physical space for contained EntityColliders to interact.
*/
class CollisionSpace {
   // memory-managed list of collider references
   ManagedList<EntityCollider> _colliders;

   // map of EntityColliderInfos
   std::unordered_map<std::string, EntityColliderInfo> _entitycolliderinfos;
    
public:
   CollisionSpace();
   CollisionSpace(CollisionSpace&& other);
   CollisionSpace(const CollisionSpace& other) = delete;
   virtual ~CollisionSpace();

   CollisionSpace& operator=(CollisionSpace&& other);
   CollisionSpace& operator=(const CollisionSpace& other) = delete;

   void addEntityCollider(EntityColliderInfo entitycolliderinfo, const char* name);

   /* Spawns a EntityCollider and returns a reference to it. */
   EntityCollider* spawnEntityCollider(const char* entitycollider_name, EntityInterface* entity, Transform transform);

   /* Erases the EntityCollider referenced by the provided EntityCollider. */
   void erase(EntityCollider* collider);

   /* Detects collision between all instances within the system via AABB method. This is done by iterating on all elements
      in a pair-wise fashion. All collided instances have their collided count incremented.
   */
   void detectCollisionAABB();

   /* Returns true if the provided collider name has been previously added to this CollisionSpace. */
   bool hasAdded(const char* collider_name);
   /* Returns the number of EntityColliders in this CollisionSpace. */
   unsigned getCount();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ProvidingEntityAllocatorInterface<T>
   Templated implementation of the ProvidingEntityAllocatorInterface, that contains a RefProvider for passing allocations
   to attached receivers. The scope of any attached RefReceiverInterface must be equal to or a subset of this instance; 
   it is undefined behavior to make calls on this instance or attached receiver instances otherwise.
   T - type allocated; must be covariant of EntityInterface
*/
template<class T>
class ProvidingEntityAllocatorInterface : public EntityAllocatorInterface {
   RefProvider<T> _refprovider;

   EntityInterface* _allocate() override {
      T* t = _providingAllocate();
      _refprovider.receiveInstance(t);
      return t;
   }

   void _onDeallocation(ScriptInterface* script) override {
      _providingOnDeallocation(script);
      _refprovider.receiveInstanceAddr(script);
   }

protected:
   virtual T* _providingAllocate() = 0;
   virtual void _providingOnDeallocation(ScriptInterface* script) = 0;

public:
   ProvidingEntityAllocatorInterface() {}

   RefProviderView<T> provider() {
      return RefProviderView<T>(&_refprovider);
   }
};

/* class GenericEntityProvider<T>
   Generic implementation of EntityProviderInterface<T>. Allocates instances of T with default constructor.
*/
template<class T>
class GenericProvidingEntityAllocator : public ProvidingEntityAllocatorInterface<T> {
   T* _providingAllocate() override { return new T; }
   void _providingOnDeallocation(ScriptInterface* script) override {}
public:
   GenericProvidingEntityAllocator() {}
};

// --------------------------------------------------------------------------------------------------------------------------

bool computeCollisionAABB(Transform transf1, Transform transf2);

/* Returns AABB distance along specified axis; if multiple components of axis are set, prioritizes x, then y, then z. 
   Returns 0 if none are set.
*/
float computeDistanceAABB(Transform transf1, Transform transf2, glm::bvec3 axis);

glm::vec3 toVec3(glm::vec2 v, float z);

glm::vec2 toVec2(glm::vec3 v);

glm::vec3 randomAngle(glm::vec3 v, float deg_range);

bool isEven(int x);

/* Returns the value with the lower absolute value. */
float absMin(float a, float b);

#endif