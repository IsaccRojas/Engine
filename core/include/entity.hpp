#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "filter.hpp"
#include "C:\dev\include\glm\glm.hpp"
#include "C:\dev\include\glm\gtx\rotate_vector.hpp"

class EntityColliderView;
class EntityExecutor;
class Entity;
class EntityManager;

/* class EntityScript
   Represents a Script that belongs to an Entity.
*/
class EntityScript : public Script {
   friend EntityExecutor;
   friend Entity;
   friend EntityManager;

   Entity* _entity;

   // called by execution environment
   void _init() override;
   void _exec() override;
   void _kill() override;
   void _update() override;

protected:
   /* Functions to be overridden by children.
      - _init() is called by runInit(). runInit() is called on spawn.
      - _exec() is called by runExec(). runExec() is called on execution, each time the Script is queued.
      - _kill() is called by runKill(). runKill() is called on erasure.
      - _update() is called by runUpdate(). runUpdate() is called when update() is called by the owning Executor.
   */
   virtual void _initEntity() = 0;
   virtual void _execEntity() = 0;
   virtual void _killEntity() = 0;
   virtual void _updateEntity() = 0;

   /* Call to handle a passed Entity and message. */
   virtual void _receive(Entity* entity, std::string message) = 0;

   /* Call invoked by EntityColliders. */
   virtual void _collide(Entity* entity) = 0;

public:
   EntityScript(EntityScript&& other);
   EntityScript();
   EntityScript(const EntityScript& other) = delete;
   virtual ~EntityScript();

   EntityScript& operator=(EntityScript&& other);
   EntityScript& operator=(const EntityScript& other) = delete;

   Entity& entity();
   void receive(Entity* entity, std::string message);
   void collide(Entity* entity);
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptView
   Contains a EntityScript reference and wraps access to EntityScript data without owning it. Invalid if the viewed Script is destroyed.
*/
class EntityScriptView : public ScriptView {
   EntityScript* _entityscript;
public:
   EntityScriptView(EntityScript* entityscript);
   void receive(Entity* entity, std::string message);
   void collide(Entity* entity);
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class EntityAllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityExecutor instance.
*/
class EntityScriptAllocatorInterface : public AllocatorInterface {
   friend EntityExecutor;
protected:
   /* Must return a heap-allocated instance of a covariant type of EntityScript. */
   virtual EntityScript* _allocate() = 0;
};

/* class GenericEntityAllocator
   A generic implementation of the EntityAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericEntityScriptAllocator : public EntityScriptAllocatorInterface {
   EntityScript* _allocate() override { return new T; } 
};

// --------------------------------------------------------------------------------------------------------------------------

// struct holding EntityScript information mapped to a name
struct EntityScriptInfo : public ScriptInfo {
   EntityScriptAllocatorInterface* _allocator;
   // default copy assignment/construction are fine
};

class EntityExecutor : public Executor {
   std::unordered_map<unsigned, EntityScript*> _entityscripts_id;

protected:
   // class to store enqueues and polymorphically spawn later
   class EntityScriptEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor* _entityexecutor;
      Entity* _entity;
   
   protected:
      // invokes the containing EntityExecutor's _spawnEntityScript() method and returns the spawned instance's reference
      virtual ScriptView spawn() override;
      EntityScriptEnqueue(EntityExecutor* entityexecutor, std::string name, int execution_queue, Entity* entity);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityExecutor)
   };

private:
   // internal variables for added EntityScript information and enqueued EntityScripts
   std::unordered_map<std::string, EntityScriptInfo> _entityscriptinfos;

protected:
   // initializes EntityScript's EntityExecutor-related fields
   void _setupEntityScript(EntityScript* entityscript, Entity* entity);
    
public:
   /* Calls init() with the provided arguments. */
   EntityExecutor(unsigned queues);
   EntityExecutor(EntityExecutor&& other);
   EntityExecutor();
   EntityExecutor(const EntityExecutor& other) = delete;
   virtual ~EntityExecutor() override;

   EntityExecutor& operator=(EntityExecutor&& other);
   EntityExecutor& operator=(const EntityExecutor& other) = delete;

   /* Initializes internal EntityExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Adds a EntityScript allocator with initialization information to this executor, allowing its given
      name to be used for future spawns.
      - EntityScriptInfo - instance of EntityScriptInfo with allocation/initialization information
      - name - name to associate with the EntityScriptInfo instance
   */
   void addEntityScript(EntityScriptInfo entityscriptinfo, const char* name);

   /* Spawns a EntityScript using a name previously added to this executor, and returns its ID. */
   EntityScriptView spawnEntityScript(const char* entityscript_name, int execution_queue, Entity* entity);

   /* Enqueues an EntityScript to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntityScript(const char* entityscript_name, int execution_queue, Entity* entity);
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ProvidedEntityScriptAllocator
   Interface that extends EntityScriptAllocatorInterface to have its allocations intercepted and stored
   by a containing Provider.
*/
template<class T>
class ProvidedEntityScriptAllocator : public ProvidedAllocator<T>, public EntityScriptAllocatorInterface {
   EntityScript* _allocate() override { return this->_allocateStore(); }
protected:
   virtual T* _allocateProvided() override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity {
   friend EntityScript;
   friend EntityExecutor;
   friend EntityManager;

   std::string _name;

   EntityManager* _entitymanager;
   std::list<Entity*>::iterator _this_iter;
   std::string _group;

   EntityScriptView _entityscriptview;
   std::vector<unsigned> _quad_ids;
   std::vector<Quad*> _quads;
   std::vector<EntityColliderView> _entitycolliderviews;

   std::unordered_map<std::string, float> _attributes1f;
   std::unordered_map<std::string, glm::vec2> _attributes2f;
   std::unordered_map<std::string, glm::vec3> _attributes3f;

   bool _script_killed;

   Transform _transform;
public:
   Entity();
   ~Entity();

   //TODO: revise copy/move semantics

   std::string& name();
   EntityManager& manager();
   EntityScriptView& entityscriptview();
   std::vector<Quad*>& quads();
   std::vector<EntityColliderView>& entitycolliderviews();
   std::unordered_map<std::string, float>& attributes1f();
   std::unordered_map<std::string, glm::vec2>& attributes2f();
   std::unordered_map<std::string, glm::vec3>& attributes3f();
   Transform& transform();
};

// --------------------------------------------------------------------------------------------------------------------------

// prototype
class CollisionSpace;

/* class EntityCollider
   Represents a physical presence capable of collision within a CollisionSpace.
*/
class EntityCollider {
   friend CollisionSpace;

   // fields maintained by owning CollisionSpace
   CollisionSpace* _collisionspace;
   std::list<EntityCollider*>::iterator _this_iter;
   FilterState _filterstate;
   
   Entity* _entity;
public:
   EntityCollider(EntityCollider&& other);
   EntityCollider();
   EntityCollider(const EntityCollider&) = delete;
   virtual ~EntityCollider();

   EntityCollider& operator=(EntityCollider&& other);
   EntityCollider& operator=(const EntityCollider&) = delete;

   // physics variables
   bool collision_enabled;
   Transform transform;
   glm::vec3 vel;

   FilterState& filterstate();
   Entity& entity();
   void step();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityColliderView
   Contains a EntityCollider reference and wraps access to EntityCollider data without owning it. Invalid if the viewed EntityCollider is destroyed.
*/
class EntityColliderView {
   friend CollisionSpace;
   EntityCollider* _collider;
public:
   EntityColliderView(EntityCollider* collider);
   bool& collision_enabled();
   Transform& transform();
   glm::vec3& vel();
};

// --------------------------------------------------------------------------------------------------------------------------

struct EntityColliderInfo {
   Transform transform;
   glm::vec3 vel;
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

   // flag to store if instance was initialized or not
   bool _initialized;
    
public:
   CollisionSpace();
   CollisionSpace(CollisionSpace&& other);
   CollisionSpace(const CollisionSpace& other) = delete;
   virtual ~CollisionSpace();

   CollisionSpace& operator=(CollisionSpace&& other);
   CollisionSpace& operator=(const CollisionSpace& other) = delete;

   /* Initializes internal CollisionSpace data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init();
   void uninit();

   void addCollider(EntityColliderInfo entitycolliderinfo, const char* name);

   /* Spawns a Collider and returns a ColliderView. */
   EntityColliderView spawnCollider(const char* name, Entity* entity);

   /* Erases the Collider referenced by the provided ColliderView. */
   void erase(EntityColliderView colliderview);

   /* Detects collision between all instances within the system via AABB method. This is done by iterating on all elements
      in a pair-wise fashion. All collided instances have their collided count incremented.
   */
   void detectCollisionAABB();

   /* Advances every internal instance one step in time. */
   void step();

   /* Returns the number of Colliders in this CollisionSpace. */
   unsigned getCount();

   /* Returns whether or not this CollisionSpace instance has been initialized or not. */
   bool initialized();
};

// --------------------------------------------------------------------------------------------------------------------------

struct EntityInfo {
   std::string group;
   std::string entityscript_name;
   int execution_queue;
   bool auto_enqueue;
   std::list<std::string> quad_names;
   std::list<std::string> entitycollider_names;
};

class EntityManager {
   // storage of entity info, mapped to names
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   // storage of entities, mapped to group names
   std::unordered_map<std::string, ManagedList<Entity>> _entities;

   EntityExecutor* _entityexecutor;
   GLEnv* _glenv;
   CollisionSpace* _collisionspace;

   bool _initialized = false;

   // can only be called from checkEntities() if entity's entityscript is killed
   void _removeEntity(Entity* entity);
public:
   EntityManager(EntityExecutor* entityexecutor, GLEnv* glenv, CollisionSpace* physspace_box);
   EntityManager(EntityManager&& other);
   EntityManager();
   EntityManager(const EntityManager& other) = delete;
   ~EntityManager();

   EntityManager& operator=(EntityManager&& other);
   EntityManager& operator=(const EntityManager& other) = delete;

   void init(EntityExecutor* entityexecutor, GLEnv* glenv, CollisionSpace* physspace_box);
   void uninit();

   void addEntity(EntityInfo info, const char* name);
   Entity* spawnEntity(const char* name, Transform transform);
   
   void checkEntities();

   std::list<Entity*>::iterator groupBegin(const char* group);

   std::list<Entity*>::iterator groupEnd(const char* group);
};

bool computeCollisionAABB(Transform transf1, Transform transf2);

glm::vec3 random_angle(glm::vec3 v, float deg_range);

bool is_even(int x);

#endif