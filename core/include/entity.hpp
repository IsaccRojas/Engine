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
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class EntityAllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityExecutor instance.
*/
class EntityAllocatorInterface {
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
   Entity *_allocate() override { return new T; }
};

/* abstract class EntityReceiver
   Interface that is used to allow reception of a generic specified type when
   implemented, via subscription to an EntityServicer of the same type.
*/

// prototype
template<typename T>
class EntityServicer;

template<class T>
class EntityReceiver {
   friend EntityServicer<T>;
   int _channel;
protected:
   virtual void _receive(T *t) = 0;
   EntityReceiver() : _channel(-1) {}
public:
   void setChannel(int channel) { _channel = channel; }
   int getChannel() { return _channel; }
};

/* class EntityServicer
Implementation of EntityAllocatorInterface that interprets the tag argument as a
"channel". Subscribed EntityReceivers will have their _receive() method invoked
whenever instances of this class have their allocator invoked. Only EntityReceivers
with a matching tag value will be passed the allocated instance of T.
*/
template<class T>
class EntityServicer : public EntityAllocatorInterface {
   std::unordered_set<EntityReceiver<T>*> _receivers;
   
   Entity *_allocate(int tag) override {
      // interpret tag as channel

      T *t = _allocateInstance();

      // if channel is non-negative, deliver instance
      if (tag >= 0) {
         // if channel matches, deliver
         for (const auto& receiver: _receivers)
               if (receiver->_channel == tag)
                  receiver->_receive(t);
      }

      return t;
   }
protected:
   virtual T *_allocateInstance() { return new T; }
public:
   void subscribe(EntityReceiver<T> *receiver) { _receivers.insert(receiver); }
};

// --------------------------------------------------------------------------------------------------------------------------

class EntityExecutor : public Executor {
public:
    // struct holding Entity information mapped to a name
    struct EntityInfo {
       EntityAllocatorInterface *_allocator;
       std::string _animation_name;
    };
    struct EntityEnqueue {
      bool _is_entity;
      glm::vec3 _pos;
    };

protected:
    // internal variables for added Entity information and enqueued Entities
    std::unordered_map<std::string, EntityInfo> _entityinfos;
    std::queue<EntityEnqueue> _entityenqueues;

    GLEnv *_glenv;
    unordered_map_string_Animation_t *_animations;

    // initializes Entity's EntityExecutor-related fields
    void _setupEntity(Entity *entity, Animation *animation);

    // spawns an Entity using a name previously added to this manager, and returns its ID
    unsigned _spawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
public:
    /* Calls init() with the provided arguments. */
    EntityExecutor(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations);
    EntityExecutor(EntityExecutor &&other);
    EntityExecutor();
    EntityExecutor(const EntityExecutor &other) = delete;
    virtual ~EntityExecutor() override;

    EntityExecutor &operator=(EntityExecutor &&other);
    EntityExecutor &operator=(const EntityExecutor &other) = delete;

    void init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations);
    void uninit();

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
    void addEntity(EntityAllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::string animation_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback);

    /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
    virtual void enqueueSpawn(const char *script_name, int execution_queue, int tag) override;
    /* Enqueues an Entity to be spawned when calling runSpawnQueue(). */
    virtual void enqueueSpawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
    /* Spawns all Entities (or sub classes) queued for spawning with enqueueSpawn(). */
    virtual std::vector<unsigned> runSpawnQueue() override;
};

#endif