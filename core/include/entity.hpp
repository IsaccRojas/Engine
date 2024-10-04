#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "animation.hpp"
#include "glenv.hpp"
#include "physspace.hpp"

typedef std::unordered_map<std::string, Animation> unordered_map_string_Animation_t;
typedef std::unordered_map<std::string, Filter> unordered_map_string_Filter_t;

class EntityExecutor;

/* class Entity
   Represents a Script that contains references to external resources, and a Transform.
*/
class Entity : public Script {
   friend EntityExecutor;

   EntityExecutor *_entityexecutor;

   // called by execution environment
   void _init() override;
   void _base() override;
   void _kill() override;

protected:
   /* Functions to be overridden by children.
      - _initEntity() is called by _init(). _init() is called on execution, only for the first time the entity is queued.
      - _baseEntity() is called by _base(). _base() is called on execution, each time the entity is queued.
      - _killEntity() is called by _kill(). _kill() is called on erasure.
   */
   virtual void _initEntity() = 0;
   virtual void _baseEntity() = 0;
   virtual void _killEntity() = 0;

public:
   Entity(Entity &&other);
   Entity();
   Entity(const Entity &other) = delete;
   virtual ~Entity();

   Entity& operator=(Entity &&other);
   Entity& operator=(const Entity &other) = delete;

   EntityExecutor &executor();
   
   Transform transform;
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class EntityAllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityExecutor instance.
*/
class EntityAllocatorInterface : public AllocatorInterface {
   friend EntityExecutor;
protected:
   /* Must return a heap-allocated instance of a covariant type of Entity. */
   virtual Entity *_allocate(int tag) = 0;
};

/* class GenericEntityAllocator
   A generic implementation of the EntityAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericEntityAllocator : public EntityAllocatorInterface {
   Entity *_allocate(int tag) override { return new T; } 
};

// --------------------------------------------------------------------------------------------------------------------------

class EntityExecutor : public Executor {
   // struct holding Entity information mapped to a name
   struct EntityInfo {
      EntityAllocatorInterface *_allocator;
      // default copy assignment/construction are fine
   };

protected:
   // class to store enqueues and polymorphically spawn later
   class EntityEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor *_entityexecutor;
   
   protected:
      Transform _transform;
      
      // invokes the containing EntityExecutor's _spawnEntity() method and returns the spawned instance's reference
      virtual Entity *spawn() override;
      EntityEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, Transform transform);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityExecutor)
   };

private:
   // internal variables for added Entity information and enqueued Entities
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   GLEnv *_glenv;
   unordered_map_string_Animation_t *_animations;
   PhysSpace<Box> *_box_space;
   PhysSpace<Sphere> *_sphere_space;
   unordered_map_string_Filter_t *_filters;

protected:
   // initializes Entity's EntityExecutor-related fields
   void _setupEntity(Entity *entity, const char *entity_name);

   // spawns an Entity using a name previously added to this EntityExecutor
   Entity *_spawnEntity(const char *entity_name, int execution_queue, int tag, Transform transform);
    
public:
   /* Calls init() with the provided arguments. */
   EntityExecutor(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysSpace<Box> *box_space, PhysSpace<Sphere> *sphere_space, unordered_map_string_Filter_t *filters);
   EntityExecutor(EntityExecutor &&other);
   EntityExecutor();
   EntityExecutor(const EntityExecutor &other) = delete;
   virtual ~EntityExecutor() override;

   EntityExecutor &operator=(EntityExecutor &&other);
   EntityExecutor &operator=(const EntityExecutor &other) = delete;

   /* Initializes internal EntityExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysSpace<Box> *box_space, PhysSpace<Sphere> *sphere_space, unordered_map_string_Filter_t *filters);
   void uninit();

   /* Adds a Entity allocator with initialization information to this manager, allowing its given
      name to be used for future spawns.
      - allocator - Reference to instance of class implementing EntityAllocatorInterface.
      - name - name to associate with the allocator
      - group - value to associate with all instances of this Entity
      - removeonkill - removes this Entity from this manager when it is killed
      - spawn_callback - function callback to call after Entity has been spawned and setup
      - remove_callback - function callback to call before Entity has been removed
   */
   void addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::function<void(Script*)> spawn_callback, std::function<void(Script*)>  remove_callback);

   /* Enqueues an Entity to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, Transform transform);

   GLEnv &glenv();
   unordered_map_string_Animation_t &animations();
   PhysSpace<Box> &boxspace();
   PhysSpace<Sphere> &spherespace();
   unordered_map_string_Filter_t &filters();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ProvidedEntityAllocator
   Interface that extends EntityAllocatorInterface to have its allocations intercepted and stored
   by a containing Provider.
*/
template<class T>
class ProvidedEntityAllocator : public ProvidedAllocator<T>, public EntityAllocatorInterface {
   Entity *_allocate(int tag) override { return this->_allocateStore(tag); }
protected:
   virtual T *_allocateProvided() override { return new T; }
};

#endif