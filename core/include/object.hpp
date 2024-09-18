#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"
#include "physenv.hpp"

typedef std::unordered_map<std::string, Filter> unordered_map_string_Filter_t;

class ObjectExecutor;

/* class Object
   Represents an Entity that contains a box. objectSetup() must be called for the
   script methods _initObject(), _baseObject(), and _killObject() to do anything.
*/
class Object : public Entity {
   friend ObjectExecutor;

   ObjectExecutor *_objectexecutor;

   // environmental references
   PhysEnv *_physenv;

   // box object
   Box *_box;

   void _initEntity() override;
   void _baseEntity() override;
   void _killEntity() override;

   void _objectRemove();
protected:
   /* Functions to be overridden by children.
      - _initObject() is called by _initEntity(). _initEntity() is called on execution, only for the first time the object is queued.
      - _baseObject() is called by _baseEntity(). _base() is called on execution, each time the object is queued.
      - _killObject() is called by _killEntity(). _kill() is called on erasure.
      - _collisionObject() is called by a PhysEnv when collision with this object's box is detected. This is supplied to the box on
      calling genBox().
   */
   virtual void _initObject() = 0;
   virtual void _baseObject() = 0;
   virtual void _killObject() = 0;
   virtual void _collisionObject(Box *box) = 0;

public:
   Object(Object &&other);
   Object();
   Object(const Object &other) = delete;
   virtual ~Object();

   Object &operator=(Object &&other);
   Object &operator=(const Object &other) = delete;

   Box *getBox();

   ObjectExecutor *getExecutor();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ObjectAllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking ObjectExecutor instance.
*/
class ObjectAllocatorInterface : public EntityAllocatorInterface {
   friend ObjectExecutor;
protected:
   /* Must return a heap-allocated instance of a covariant type of Object. */
   virtual Object *_allocate(int tag) = 0;
};

/* class GenericObjectAllocator
   A generic implementation of the ObjectAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericObjectAllocator : public ObjectAllocatorInterface {
    Object *_allocate(int tag) override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

class ObjectExecutor : public EntityExecutor {
   // struct holding Object information mapped to a name
   struct ObjectInfo {
      ObjectAllocatorInterface *_allocator;
      std::string _filter_name;
   };

// class to store enqueues and polymorphically spawn later
   class ObjectEnqueue : public EntityEnqueue {
      friend ObjectExecutor;
      ObjectExecutor *_objectexecutor;
      Object *spawn() override;
      ObjectEnqueue(ObjectExecutor *objectexecutor, std::string name, int execution_queue, int tag, glm::vec3 pos);
   };

   // internal variables for added Object information and enqueued Entities
   std::unordered_map<std::string, ObjectInfo> _objectinfos;

   PhysEnv *_physenv;
   unordered_map_string_Filter_t *_filters;

   // initializes Object's ObjectExecutor-related fields
   void _setupObject(Object *object, const char *object_name);

   // spawns an Object using a name previously added to this ObjectExecutor, and returns its ID
   Object *_spawnObject(const char *object_name, int execution_queue, int tag, glm::vec3 pos);
    
public:
   /* Calls init() with the provided arguments. */
   ObjectExecutor(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters);
   ObjectExecutor(ObjectExecutor &&other);
   ObjectExecutor();
   ObjectExecutor(const ObjectExecutor &other) = delete;
   ~ObjectExecutor() override;

   ObjectExecutor &operator=(ObjectExecutor &&other);
   ObjectExecutor &operator=(const ObjectExecutor &other) = delete;

   /* Initializes internal EntityExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters);
   void uninit();

   /* Adds a Object allocator with initialization information to this manager, allowing its given
      name to be used for future spawns.
      - allocator - Reference to instance of class implementing ObjectAllocatorInterface.
      - name - name to associate with the allocator
      - group - value to associate with all instances of this Object
      - removeonkill - removes this Object from this manager when it is killed
      - animation_name - animation to associate with this name
      - spawn_callback - function callback to call after Object has been spawned and setup
      - remove_callback - function callback to call before Object has been removed
   */
   void addObject(ObjectAllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, std::string animation_name, std::string filter_name, std::function<void(Script*)> spawn_callback, std::function<void(Script*)>  remove_callback);

   /* Enqueues an Object to be spawned when calling runSpawnQueue(). */
   void enqueueSpawnObject(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ProvidedObjectAllocator
   Interface that extends ObjectAllocatorInterface to have its allocations intercepted and stored
   by a containing Provider.
*/
template<class T>
class ProvidedObjectAllocator : public ProvidedEntityAllocator<T>, public ObjectAllocatorInterface {
   Object *_allocate(int tag) override { return this->_allocateStore(tag); }
protected:
   virtual T *_allocateProvided() = 0;
};

#endif