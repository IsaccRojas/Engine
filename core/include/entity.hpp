#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "glenv.hpp"
#include "script.hpp"

typedef std::unordered_map<std::string, Animation> unordered_map_string_Animation_t;

class EntityExecutor;

/* class Entity
   Represents a Script that contains a Quad. entitySetup() must be called for the
   script methods _initEntity(), _baseEntity(), and _killEntity() to do anything.
*/
class Entity : public Script {
   friend EntityExecutor;

   EntityExecutor *_entityexecutor;
   
   // environmental references
   GLEnv *_glenv;

   // sprite objects
   Quad *_quad;
   Frame *_frame;

   // sprite variables/flags
   int _quad_id;
   bool _first_step;

   // controllable variables
   AnimationState _animationstate;

   // called by execution environment
   void _init() override;
   void _base() override;
   void _kill() override;

   void _entityRemove();
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

   void stepAnim();

   AnimationState &getAnimState();
   Quad *getQuad();

   EntityExecutor *getExecutor();
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
      std::string _animation_name;
   };

protected:
// class to store enqueues and polymorphically spawn later
   class EntityEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor *_entityexecutor;
   protected:
      glm::vec3 _pos;
      virtual Entity *spawn() override;
      EntityEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, glm::vec3 pos);
   };

private:
   // internal variables for added Entity information and enqueued Entities
   std::unordered_map<std::string, EntityInfo> _entityinfos;

   GLEnv *_glenv;
   unordered_map_string_Animation_t *_animations;

protected:
   // initializes Entity's EntityExecutor-related fields
   void _setupEntity(Entity *entity, const char *entity_name);

   // spawns an Entity using a name previously added to this EntityExecutor
   Entity *_spawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
    
public:
   EntityExecutor(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations);
   EntityExecutor(EntityExecutor &&other);
   EntityExecutor();
   EntityExecutor(const EntityExecutor &other) = delete;
   virtual ~EntityExecutor() override;

   EntityExecutor &operator=(EntityExecutor &&other);
   EntityExecutor &operator=(const EntityExecutor &other) = delete;

   /* Adds a Entity allocator with initialization information to this manager, allowing its given
      name to be used for future spawns.
      - allocator - Reference to instance of class implementing EntityAllocatorInterface.
      - name - name to associate with the allocator
      - group - value to associate with all instances of this Entity
      - removeonkill - removes this Entity from this manager when it is killed
      - animation_name - animation to associate with this name
      - spawn_callback - function callback to call after Entity has been spawned and setup
      - remove_callback - function callback to call before Entity has been removed
   */
   void addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::string animation_name, std::function<void(Script*)> spawn_callback, std::function<void(Script*)>  remove_callback);

   /* Enqueues an Entity to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
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
   virtual T *_allocateProvided() = 0;
};

#endif