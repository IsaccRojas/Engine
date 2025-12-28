#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "physspace.hpp"

class EntityExecutor;

/* class EntityScript
   Represents a Script that belongs to an Entity.
*/
class EntityScript : public Script {
   friend EntityExecutor;

   // called by execution environment
   void _init() override;
   void _base() override;
   void _kill() override;

protected:
   /* Functions to be overridden by children.
      - _initEntity() is called by _init(). _init() is called on execution, only for the first time the EntityScript is queued.
      - _baseEntity() is called by _base(). _base() is called on execution, each time the EntityScript is queued.
      - _killEntity() is called by _kill(). _kill() is called on erasure.
   */
   virtual void _initEntity() = 0;
   virtual void _baseEntity() = 0;
   virtual void _killEntity() = 0;

public:
   EntityScript(EntityScript &&other);
   EntityScript();
   EntityScript(const EntityScript &other) = delete;
   virtual ~EntityScript();

   EntityScript& operator=(EntityScript &&other);
   EntityScript& operator=(const EntityScript &other) = delete;
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
   virtual EntityScript *_allocate(int tag) = 0;
};

/* class GenericEntityAllocator
   A generic implementation of the EntityAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericEntityScriptAllocator : public EntityScriptAllocatorInterface {
   EntityScript *_allocate(int tag) override { return new T; } 
};

// --------------------------------------------------------------------------------------------------------------------------

class EntityExecutor : public Executor {
   // struct holding EntityScript information mapped to a name
   struct EntityScriptInfo {
      EntityScriptAllocatorInterface *_allocator;
      // default copy assignment/construction are fine
   };

protected:
   // class to store enqueues and polymorphically spawn later
   class EntityScriptEnqueue : public ScriptEnqueue {
      friend EntityExecutor;
      EntityExecutor *_entityexecutor;
   
   protected:
      Transform _transform;
      
      // invokes the containing EntityExecutor's _spawnEntity() method and returns the spawned instance's reference
      virtual unsigned spawn() override;
      EntityScriptEnqueue(EntityExecutor *entityexecutor, std::string name, int execution_queue, int tag, Transform transform);
      // default copy assignment/construction are fine (copying implies another enqueue in the same EntityExecutor)
   };

private:
   // internal variables for added EntityScript information and enqueued EntityScripts
   std::unordered_map<std::string, EntityScriptInfo> _entityscriptinfos;

protected:
   // initializes EntityScript's EntityExecutor-related fields
   void _setupEntityScript(EntityScript *entityscript);
    
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
   void addEntityScript(EntityScriptAllocatorInterface *allocator, const char *name, std::function<void(Script*)> spawn_callback, std::function<void(Script*)>  remove_callback);

   /* Spawns a EntityScript using a name previously added to this manager, and returns its ID. */
   unsigned spawnEntityScript(const char *entityscript_name, int execution_queue, int tag, Transform transform);

   /* Enqueues an EntityScript to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnEntityScript(const char *entityscript_name, int execution_queue, int tag, Transform transform);
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ProvidedEntityScriptAllocator
   Interface that extends EntityScriptAllocatorInterface to have its allocations intercepted and stored
   by a containing Provider.
*/
template<class T>
class ProvidedEntityScriptAllocator : public ProvidedAllocator<T>, public EntityScriptAllocatorInterface {
   Entity *_allocate(int tag) override { return this->_allocateStore(tag); }
protected:
   virtual T *_allocateProvided() override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

struct Entity {
    unsigned _group;
    unsigned _entity_manager_id;
    std::unordered_map<const char*, float> _data_values;
    std::vector<unsigned> _script_ids;
public:
    std::vector<Quad*> _quad_ids;
    std::vector<Box*> _box_ids;
};

/*
struct Scheme;
typedef std::unordered_map<std::string, Scheme> unordered_map_string_Scheme_t;

struct ScriptArgs {
    const char *script_name;
    int execution_queue;
    int tag;
};
struct QuadArgs {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec4 color;
    DrawType type;
    const char *animation_name;
    glm::vec3 texpos;
    glm::vec2 texsize;
    GLfloat innerrad;
};
struct BoxArgs {
    Transform transf;
    glm::vec3 vel;
    std::function<void(Box*)> callback;
    const char *filter_name;
};
struct Scheme {
    std::list<ScriptArgs> _script_args;
    std::list<QuadArgs> _quad_args;
    std::list<BoxArgs> _box_args;
};

class Manager {
    Executor *_executor;
    GLEnv * _glenv;
    PhysSpace<Box> *_physspace_box;

    unordered_map_string_Scheme_t _schemes;

    bool _initialized = false;
public:
    Manager(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box);
    Manager();
    void init(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box);
    void uninit();
    void addScheme(Scheme s, const char *name);
    void instScheme(const char *name);
};
*/

#endif