#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "filter.hpp"
#include "C:\dev\include\glm\glm.hpp"
#include "C:\dev\include\glm\gtx\rotate_vector.hpp"

class EntityScriptExecutor;
class EntityCollider;
class CollisionSpace;
class Entity;
class EntityScriptAllocatorInterface;
class EntityManager;

/* class EntityScriptInterface
   Represents a ScriptInterface that belongs to an Entity.
*/
class EntityScriptInterface : public ScriptInterface {
   friend EntityScriptExecutor;

   Entity* _entity;

   // called by execution environment
   void _init() override;
   void _exec() override;
   void _kill() override;
   void _update() override;

protected:
   /* Functions to be overridden by children.
      - _init() is called by runInit(). runInit() is called on spawn.
      - _exec() is called by runExec(). runExec() is called on execution, each time the script is queued.
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

   EntityScriptInterface(EntityScriptInterface&& other);
   EntityScriptInterface();

public:
   EntityScriptInterface(const EntityScriptInterface& other) = delete;
   virtual ~EntityScriptInterface();

   EntityScriptInterface& operator=(EntityScriptInterface&& other);
   EntityScriptInterface& operator=(const EntityScriptInterface& other) = delete;

   Entity& entity();
   void receive(Entity* entity, std::string message);
   void collide(Entity* entity);
};

// --------------------------------------------------------------------------------------------------------------------------

// struct holding EntityScript information mapped to a name
struct EntityScriptInfo {
   EntityScriptAllocatorInterface* allocator;
   int preferred_queue;
   std::function<void(ScriptInterface*)> spawn_callback;
   std::function<void(ScriptInterface*)> remove_callback;
   // default copy assignment/construction are fine
};

class EntityScriptExecutor : public ScriptExecutor {
   std::unordered_map<unsigned, EntityScriptInterface*> _entityscripts_id;

protected:
   // class to store enqueues and polymorphically spawn later
   class EntityScriptEnqueue : public ScriptEnqueue {
      friend EntityScriptExecutor;
      EntityScriptExecutor* _entityscriptexecutor;
      Entity* _entity;
   
   protected:
      // invokes the containing EntityScriptExecutor's _spawnEntityScript() method and returns the spawned instance's reference
      virtual ScriptInterface* spawn() override;
      EntityScriptEnqueue(EntityScriptExecutor* entityscriptexecutor, std::string name, Entity* entity);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityScriptExecutor)
   };

private:
   // internal variables for added EntityScriptInterface information and enqueued EntityScriptInterfaces
   std::unordered_map<std::string, EntityScriptInfo> _entityscriptinfos;

protected:
   // initializes EntityScriptInterface's EntityScriptExecutor-related fields
   void _setupEntityScript(EntityScriptInterface* entityscript, Entity* entity);
    
public:
   /* Calls init() with the provided arguments. */
   EntityScriptExecutor(unsigned queues);
   EntityScriptExecutor(EntityScriptExecutor&& other);
   EntityScriptExecutor();
   EntityScriptExecutor(const EntityScriptExecutor& other) = delete;
   virtual ~EntityScriptExecutor() override;

   EntityScriptExecutor& operator=(EntityScriptExecutor&& other);
   EntityScriptExecutor& operator=(const EntityScriptExecutor& other) = delete;

   /* Initializes internal EntityScriptExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Adds an EntityScriptInterface allocator with initialization information to this executor, allowing its given
      name to be used for future spawns.
      - EntityScriptInfo - instance of EntityScriptInfo with allocation/initialization information
      - name - name to associate with the EntityScriptInfo instance
   */
   void addEntityScript(EntityScriptInfo entityscriptinfo, const char* name);

   /* Spawns a EntityScript using a name previously added to this executor, and returns its ID. */
   EntityScriptInterface* spawnEntityScript(const char* entityscript_name, Entity* entity);

   /* Enqueues an EntityScript to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntityScript(const char* entityscript_name, Entity* entity);
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity {
   friend EntityManager;

   std::string _entity_name;

   EntityManager* _entitymanager;
   std::list<Entity*>::iterator _this_iter;
   std::string _group;

   EntityScriptInterface* _entityscript;
   std::vector<unsigned> _quad_ids;
   std::vector<Quad*> _quads;
   std::vector<EntityCollider*> _entitycolliders;

   bool _script_kill_started;

   Transform _globaltransform;

   Entity();

public:
   ~Entity();

   //TODO: revise copy/move semantics

   /* Checks if script was killed, enabling its removal from its manager. */
   void checkScriptStatus();

   const char* getName();
   EntityScriptInterface* entityscript();
   std::vector<Quad*>& quads();
   std::vector<EntityCollider*>& entitycolliders();
   Transform& globaltransform();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class EntityAllocatorInterface
   Is used to invoke _allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityScriptExecutor instance.

   Stores a reference that can be checked against for existence by subtypes.
*/
class EntityScriptAllocatorInterface : public ScriptAllocatorInterface {
   friend EntityScriptExecutor;

protected:
   // must return a heap-allocated instance of a covariant type of EntityScriptInterface
   virtual EntityScriptInterface* _allocate() = 0;

   // called on deallocation; should not deallocate anything
   virtual void _onDeallocation(ScriptInterface* script) = 0;

   EntityScriptAllocatorInterface();
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
   glm::vec3 _base_pos;
   glm::vec3 _base_scale;
   glm::vec3 _pos;
   glm::vec3 _scale;
   Transform _prev_applied_transform;

   Entity* _entity;

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

   /* Get last applied Transform */
   Transform getPrevAppliedTransform();

   FilterState& filterstate();
   Entity& entity();
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

   /* Spawns a EntityCollider and returns a reference to it. */
   EntityCollider* spawnCollider(const char* name, Entity* entity, Transform transform);

   /* Erases the EntityCollider referenced by the provided EntityCollider. */
   void erase(EntityCollider* collider);

   /* Detects collision between all instances within the system via AABB method. This is done by iterating on all elements
      in a pair-wise fashion. All collided instances have their collided count incremented.
   */
   void detectCollisionAABB();

   /* Returns the number of EntityColliders in this CollisionSpace. */
   unsigned getCount();

   /* Returns whether or not this CollisionSpace instance has been initialized or not. */
   bool initialized();
};

// --------------------------------------------------------------------------------------------------------------------------

struct EntityInfo {
   std::string group;
   std::string entityscript_name;
   bool auto_enqueue;
   std::list<std::string> quad_names;
   std::list<std::string> entitycollider_names;
};

class EntityManager {
   // storage of entity info, mapped to names
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   // storage of entity group names
   std::list<std::string> _entity_group_names;

   // storage of entities, mapped to group names
   std::unordered_map<std::string, ManagedList<Entity>> _entities;

   EntityScriptExecutor* _entityscriptexecutor;
   GLEnv* _glenv;
   CollisionSpace* _collisionspace;

   bool _initialized = false;

   // can only be called from checkEntities() if entity's entityscript is killed
   void _removeEntity(Entity* entity);
   
public:
   EntityManager(EntityScriptExecutor* entityscriptexecutor, GLEnv* glenv, CollisionSpace* physspace_box);
   EntityManager(EntityManager&& other);
   EntityManager();
   EntityManager(const EntityManager& other) = delete;
   ~EntityManager();

   EntityManager& operator=(EntityManager&& other);
   EntityManager& operator=(const EntityManager& other) = delete;

   void init(EntityScriptExecutor* entityscriptexecutor, GLEnv* glenv, CollisionSpace* physspace_box);
   void uninit();

   void addEntity(EntityInfo info, const char* name);
   Entity* spawnEntity(const char* name, Transform transform);
   
   void checkEntities();

   std::list<Entity*>::iterator groupBegin(const char* group);

   std::list<Entity*>::iterator groupEnd(const char* group);
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptProviderInterface<T, ...BaseTs>
   Templated implementation of the EntityScriptAllocatorInterface, that contains a RefProvider for passing allocations
   to attached receivers. The scope of any attached RefReceiverInterface must be equal to or a subset of this instance; 
   it is undefined behavior to make calls on this instance or attached receiver instances otherwise.
   T - type allocated; must be covariant of EntityScriptInterface
   ...BaseTs - types to be supported to be passed to attached RefReceiverInterfaces; must be covariant of T and thus of EntityScriptInterface
*/
template<class T, class ...BaseTs>
class EntityScriptProviderInterface : public EntityScriptAllocatorInterface {
    RefProvider<T, BaseTs...> _refprovider;

    EntityScriptInterface* _allocate() override {
        T* t = _providerAllocate();
        _refprovider.receiveInstance(t);
        return t;
    }

    void _onDeallocation(ScriptInterface* script) override {
        _providerOnDeallocation(script);
        _refprovider.receiveInstanceAddr(script);
    }

protected:
    virtual T* _providerAllocate() = 0;
    virtual void _providerOnDeallocation(ScriptInterface* script) = 0;

public:
    EntityScriptProviderInterface() {}
    template<class U>
    void attach(RefReceiverInterface<U>* refreceiver) {
        _refprovider.attach(refreceiver);
    }

    template<class U>
    void detach(RefReceiverInterface<U>* refreceiver) {
        _refprovider.detach(refreceiver);
    }
};

/* class GenericEntityScriptProvider<T>
   Generic implementation of EntityScriptProviderInterface<T>. Allocates instances of T with default constructor.
*/
template<class T>
class GenericEntityScriptProvider : public EntityScriptProviderInterface<T, T> {
    T* _providerAllocate() override { return new T; }
    void _providerOnDeallocation(ScriptInterface* script) override {}
public:
    GenericEntityScriptProvider() {}
};

// --------------------------------------------------------------------------------------------------------------------------

bool computeCollisionAABB(Transform transf1, Transform transf2);

glm::vec3 random_angle(glm::vec3 v, float deg_range);

bool is_even(int x);

#endif