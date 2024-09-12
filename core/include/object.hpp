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
    int _box_id;

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

    struct ObjectValues {
       Object *_object_ref;
    };

   // class to store enqueues and polymorphically spawn later
    class ObjectEnqueue : public EntityEnqueue {
       friend ObjectExecutor;
       ObjectExecutor *_objectexecutor;
       int spawn() override;
       ObjectEnqueue(ObjectExecutor *objectexecutor, std::string name, int execution_queue, int tag, glm::vec3 pos);
    };

    // internal variables for added Object information and enqueued Entities
    std::unordered_map<std::string, ObjectInfo> _objectinfos;
    std::vector<ObjectValues> _objectvalues;

    PhysEnv *_physenv;
    unordered_map_string_Filter_t *_filters;

    // initializes Object's ObjectExecutor-related fields (should already have an ID by the time this is invoked)
    void _setupObject(unsigned id, Object *object, const char *object_name);

    // spawns an Object using a name previously added to this manager, and returns its ID
    unsigned _spawnObject(const char *object_name, int execution_queue, int tag, glm::vec3 pos);
    
public:
    /* Calls init() with the provided arguments. */
    ObjectExecutor(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters);
    ObjectExecutor(ObjectExecutor &&other);
    ObjectExecutor();
    ObjectExecutor(const ObjectExecutor &other) = delete;
    ~ObjectExecutor() override;

    ObjectExecutor &operator=(ObjectExecutor &&other);
    ObjectExecutor &operator=(const ObjectExecutor &other) = delete;

    void init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations, PhysEnv *physenv, unordered_map_string_Filter_t *filters);
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
    void addObject(ObjectAllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, std::string animation_name, std::string filter_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback);

    /* Enqueues an Object to be spawned when calling runSpawnQueue(). */
    void enqueueSpawnObject(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);

    Object *getObject(unsigned id);
};

// --------------------------------------------------------------------------------------------------------------------------

// prototype
template<typename T>
class ObjectProvider;

/* abstract class ObjectReceiver
Interface that is used to allow reception of a generic specified type when
implemented, via subscription to a ObjectProvider of the same type.
*/
template<class T>
class ObjectReceiver {
   friend ObjectProvider<T>;
   ObjectProvider<T> *_provider;
   int _channel;
   bool _reception;
protected:
   virtual void _receive(T *t) = 0;
   ObjectReceiver() : _provider(nullptr), _channel(-1), _reception(false) {}
   virtual ~ObjectReceiver() {
      if (_provider)
         _provider->unsubscribe(this);
   }
public:
   void setChannel(int channel) { _channel = channel; }
   void enableReception(bool state) { _reception = state; }
   int getChannel() { return _channel; }
};

/* class ObjectProvider
Implementation of ObjectAllocatorInterface that interprets the tag argument as a
"channel". Subscribed ObjectReceivers will have their _receive() method invoked
whenever instances of this class have their allocator invoked. Only ObjectReceivers
with a matching tag value will be passed the allocated instance of T.
*/
template<class T>
class ObjectProvider : public ObjectAllocatorInterface {
   std::unordered_set<ObjectReceiver<T>*> _receivers;
   
   Object *_allocate(int tag) override {
      // interpret tag as channel

      T *t = _allocateInstance();

      // if channel is non-negative, deliver instance
      if (tag >= 0) {
         // if channel matches, deliver
         for (const auto& receiver: _receivers)
            if (receiver->_reception)
               if (receiver->_channel == tag)
                     receiver->_receive(t);
      }

      return t;
   }
   
protected:
   virtual T *_allocateInstance() { return new T; }
public:
   void subscribe(ObjectReceiver<T> *receiver) {
      _receivers.insert(receiver);
      receiver->_provider = this;
   }
   void unsubscribe(ObjectReceiver<T> *receiver) {
      _receivers.erase(receiver);
      receiver->_provider = nullptr;
   }
};

#endif