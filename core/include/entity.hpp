#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "filter.hpp"
#include "glm/glm.hpp"
#include "glm\gtx\rotate_vector.hpp"

typedef std::unordered_map<std::string, Filter> unordered_map_string_Filter_t;

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

   Entity *_entity;

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
   virtual void _receive(Entity *entity, std::string message) = 0;

   /* Call invoked by EntityColliders. */
   virtual void _collide(Entity *entity) = 0;

public:
   EntityScript(EntityScript &&other);
   EntityScript();
   EntityScript(const EntityScript &other) = delete;
   virtual ~EntityScript();

   EntityScript& operator=(EntityScript &&other);
   EntityScript& operator=(const EntityScript &other) = delete;

   Entity &entity();
   void receive(Entity *entity, std::string message);
   void collide(Entity *entity);
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptView
   Contains a EntityScript reference and wraps access to EntityScript data without owning it. Invalid if the viewed Script is destroyed.
*/
class EntityScriptView : public ScriptView {
   EntityScript *_entityscript;
public:
   EntityScriptView(EntityScript *entityscript);
   void receive(Entity *entity, std::string message);
   void collide(Entity *entity);
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
   virtual EntityScript *_allocate() = 0;
};

/* class GenericEntityAllocator
   A generic implementation of the EntityAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericEntityScriptAllocator : public EntityScriptAllocatorInterface {
   EntityScript *_allocate() override { return new T; } 
};

// --------------------------------------------------------------------------------------------------------------------------

class EntityExecutor : public Executor {
   // struct holding EntityScript information mapped to a name
   struct EntityScriptInfo {
      EntityScriptAllocatorInterface *_allocator;
      // default copy assignment/construction are fine
   };

   std::unordered_map<unsigned, EntityScript*> _entityscripts_id;

protected:
   // class to store enqueues and polymorphically spawn later
   class EntityScriptEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor *_entityexecutor;
      Entity *_entity;
   
   protected:
      // invokes the containing EntityExecutor's _spawnEntityScript() method and returns the spawned instance's reference
      virtual ScriptView spawn() override;
      EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, Entity *entity);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityExecutor)
   };

private:
   // internal variables for added EntityScript information and enqueued EntityScripts
   std::unordered_map<std::string, EntityScriptInfo> _entityscriptinfos;

protected:
   // initializes EntityScript's EntityExecutor-related fields
   void _setupEntityScript(EntityScript *entityscript, Entity *entity);
    
public:
   /* Calls init() with the provided arguments. */
   EntityExecutor(unsigned queues);
   EntityExecutor(EntityExecutor &&other);
   EntityExecutor();
   EntityExecutor(const EntityExecutor &other) = delete;
   virtual ~EntityExecutor() override;

   EntityExecutor &operator=(EntityExecutor &&other);
   EntityExecutor &operator=(const EntityExecutor &other) = delete;

   /* Initializes internal EntityExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Adds a EntityScript allocator with initialization information to this executor, allowing its given
      name to be used for future spawns.
      - allocator - Reference to instance of class implementing EntityScriptAllocatorInterface.
      - name - name to associate with the allocator
      - removeonkill - removes this EntityScript from this executor when it is killed
      - spawn_callback - function callback to call after EntityScript has been spawned and setup
      - remove_callback - function callback to call before EntityScript has been removed
   */
   void addEntityScript(EntityScriptAllocatorInterface *allocator, const char *name, std::function<void(ScriptView)> spawn_callback, std::function<void(ScriptView)>  remove_callback);

   /* Spawns a EntityScript using a name previously added to this executor, and returns its ID. */
   EntityScriptView spawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity);

   /* Enqueues an EntityScript to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, Entity *entity);
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ProvidedEntityScriptAllocator
   Interface that extends EntityScriptAllocatorInterface to have its allocations intercepted and stored
   by a containing Provider.
*/
template<class T>
class ProvidedEntityScriptAllocator : public ProvidedAllocator<T>, public EntityScriptAllocatorInterface {
   EntityScript *_allocate() override { return this->_allocateStore(); }
protected:
   virtual T *_allocateProvided() override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity {
   friend EntityScript;
   friend EntityExecutor;
   friend EntityManager;

   EntityManager *_entitymanager;
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

public:
   Entity();
   ~Entity();

   //TODO: revise copy/move semantics

   EntityManager &manager();
   EntityScriptView &entityscriptview();
   std::vector<Quad*> &quads();
   std::vector<EntityColliderView> &entitycolliderviews();
   std::unordered_map<std::string, float> &attributes1f();
   std::unordered_map<std::string, glm::vec2> &attributes2f();
   std::unordered_map<std::string, glm::vec3> &attributes3f();
};

// --------------------------------------------------------------------------------------------------------------------------

struct Transform {
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(0.0f);
    // default copy assignment/construction are fine
};

// prototype
class CollisionSpace;

/* class EntityCollider
   Represents a physical presence capable of collision within a CollisionSpace.
*/
class EntityCollider {
    friend CollisionSpace;

    // fields maintained by owning CollisionSpace
    CollisionSpace *_collisionspace;
    std::list<EntityCollider*>::iterator _this_iter;
    FilterState _filterstate;

    bool _collision_enabled;
    
    Entity *_entity;
public:
    EntityCollider(EntityCollider &&other);
    EntityCollider();
    EntityCollider(const EntityCollider&) = delete;
    virtual ~EntityCollider();

    EntityCollider &operator=(EntityCollider &&other);
    EntityCollider &operator=(const EntityCollider&) = delete;

    // physics variables
    Transform transform;
    glm::vec3 vel;

    FilterState &filterstate();
    Entity &entity();
    void step();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityColliderView
   Contains a EntityCollider reference and wraps access to EntityCollider data without owning it. Invalid if the viewed EntityCollider is destroyed.
*/
class EntityColliderView {
    friend CollisionSpace;
    EntityCollider *_collider;
public:
    EntityColliderView(EntityCollider *collider);
    Transform &transform();
    glm::vec3 &vel();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class CollisionSpace
   Encapsulates a physical space for contained Colliders to interact.
*/
class CollisionSpace {
    // memory-managed list of Collider references
    ManagedList<EntityCollider> _colliders;

    // reference to map of filters
    unordered_map_string_Filter_t *_filters;

    // flag to store if instance was initialized or not
    bool _initialized;
    
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
    EntityColliderView spawnCollider(Transform transform, glm::vec3 vel, const char *filter_name, Entity *entity);

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

struct EntityScriptArgs{
   std::string entityscript_name;
   int execution_queue;
};
struct QuadArgs {
   glm::vec3 pos;
   glm::vec3 scale;
   glm::vec4 color;
   DrawType type;
   std::string animation_name;
   glm::vec3 texpos;
   glm::vec2 texsize;
   GLfloat innerrad;
};
struct EntityColliderArgs {
   Transform transf;
   glm::vec3 vel;
   std::string filter_name;
};

struct EntityInfo {
   EntityScriptArgs _entityscript_args;
   std::list<QuadArgs> _quad_args;
   std::list<EntityColliderArgs> _entitycollider_args;
   std::string _group;
};

class EntityManager {
   // storage of entity info, mapped to names
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   // storage of entities, mapped to group names
   std::unordered_map<std::string, ManagedList<Entity>> _entities;

   EntityExecutor *_entityexecutor;
   GLEnv * _glenv;
   CollisionSpace *_collisionspace;

   bool _initialized = false;

   // can only be called from checkEntities() if entity's entityscript is killed
   void _removeEntity(Entity *entity);
public:
   EntityManager(EntityExecutor *entityexecutor, GLEnv *glenv, CollisionSpace *physspace_box);
   EntityManager(EntityManager &&other);
   EntityManager();
   EntityManager(const EntityManager &other) = delete;
   ~EntityManager();

   EntityManager &operator=(EntityManager &&other);
   EntityManager &operator=(const EntityManager &other) = delete;

   void init(EntityExecutor *entityexecutor, GLEnv *glenv, CollisionSpace *physspace_box);
   void uninit();

   void addEntity(EntityInfo info, const char *name);
   Entity *spawnEntity(const char *name);
   
   void checkEntities();

   std::list<Entity*>::iterator groupBegin(const char *group);

   std::list<Entity*>::iterator groupEnd(const char *group);
};

bool computeCollisionAABB(Transform transf1, Transform transf2);

glm::vec3 random_angle(glm::vec3 v, float deg_range);

bool is_even(int x);

#endif