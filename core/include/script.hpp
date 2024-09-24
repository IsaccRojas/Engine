#ifndef SCRIPT_HPP_
#define SCRIPT_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <glm\glm.hpp>
#include <unordered_set>

#include "commonexcept.hpp"
#include "managedlist.hpp"
#include "managedqueue.hpp"
#include "util.hpp"

// prototype
class Executor;

/* class Script
   Represents a runnable script by an owning Executor instance.
   The owning Executor will call runInit(), runBase(), and runKill() as needed, and
   expects _init(), _base(), and _kill() to be implemented by children. 
*/
class Script {
   friend Executor;

   // fields maintained by owning Executor
   Executor *_executor;
   unsigned _executor_id;
   std::list<Script*>::iterator _this_iter;
   int _last_execqueue;
   bool _removeonkill;
   bool _initialized;
   bool _killed;
   bool _exec_enqueued;
   bool _kill_enqueued; 
   std::string _script_name;

   // settable integer usable for identification
   int _group;

   // removes the Script from its owning Executor
   void _scriptErase();
protected:
   /* Functions to be overridden by children.
      - _init() is called by runInit(). _runInit() is called on execution, only for the first time the Script is queued.
      - _base() is called by runBase(). _runBase() is called on execution, each time the Script is queued.
      - _kill() is called by runKill(). _runKill() is called on erasure.
   */
   virtual void _init() = 0;
   virtual void _base() = 0;
   virtual void _kill() = 0;

public:
   Script(Script &&other);
   Script();
   Script(const Script&) = delete;
   virtual ~Script();

   Script& operator=(Script &&other);
   Script& operator=(const Script&) = delete;

/* Functions wrapping the virtual versions of the same method, which are directly called by the Executor.
      - runInit() is called on execution, only for the first time the Script is queued.
      - runBase() is called on execution, each time the Script is queued.
      - runKill() is called on erasure.
   */
   void runInit();
   void runBase();
   void runKill();

   /* Enqueues the Script for execution.
      - queue - queue to enqueue into
   */
   void enqueueExec(unsigned queue);

   /* Kills the Script. */
   void enqueueKill();

   /* Sets various internal flags used by Executors to control state. Can be set manually to manipulate
      execution behavior.
   */
   int getLastExecQueue();
   bool getInitialized();
   bool getKilled();
   bool getExecEnqueued();
   bool getKillEnqueued();
   const char *getName();
   int getGroup();
   Executor &executor();
   unsigned getExecutorID();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class AllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking Executor instance.
*/
class Executor;
class AllocatorInterface {
   friend Executor;
protected:
   /* Must return a heap-allocated instance of a covariant type of Script. */
   virtual Script *_allocate(int tag) = 0;
};

/* class GenericAllocator
   A generic implementation of the AllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericAllocator : public AllocatorInterface {
   Script *_allocate(int tag) override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class Executor
   Encapsulates an execution environment for queue-able, inheritable Script instances.
   Uses queues to control execution and erasure of Scripts. A variable number of queues 
   for execution can be specified.
*/
class Executor {
   // struct holding Script information mapped to a name
   struct ScriptInfo {
      int _group;
      bool _removeonkill;
      AllocatorInterface *_allocator;
      std::function<void(Script*)> _spawn_callback;
      std::function<void(Script*)> _remove_callback;
   };

protected:
   // class to store enqueues and polymorphically spawn later
   class ScriptEnqueue {
      friend Executor;
      Executor *_executor;
   protected:
      std::string _name;
      int _execution_queue;
      int _tag;
      virtual Script *spawn();
      ScriptEnqueue(Executor *executor, std::string name, int execution_queue, int tag);
   public:
      virtual ~ScriptEnqueue();
   };

private:
   /* Script data structures */
   // memory-managed list of Script references and IntGenerator to provide Scripts with unique identifiers
   ManagedList<Script> _scripts;
   IntGenerator _intgen;

   // internal variables for added script information and active scripts
   std::unordered_map<std::string, ScriptInfo> _scriptinfos;
   ManagedQueue<ScriptEnqueue> _scriptenqueues;

   // queues of Scripts to be executed; swapped on execution
   struct QueuePair {
      std::queue<Script*> _push_execqueue;
      std::queue<Script*> _run_execqueue;
   };
   std::vector<QueuePair> _queuepairs;
   
   // queue of Scripts to be erased
   std::queue<Script*> _push_killqueue;
   std::queue<Script*> _run_killqueue;

   bool _initialized;

protected:
   // initializes Script's Executor-related fields
   void _setupScript(Script *script, const char *script_name, int execution_queue);

   // spawns a Script using a name previously added to this manager, and returns its ID
   Script *_spawnScript(const char *script_name, int execution_queue, int tag);

   // pushes an enqueue
   void _pushSpawnEnqueue(ScriptEnqueue *enqueue);

   // checks if the provided Script belongs to this Executor; throws if not
   void _checkOwned(Script *script);

public:
   /* Calls init() with the provided arguments. */
   Executor(unsigned queues);
   Executor();
   Executor(Executor &&other);
   Executor(const Executor &other) = delete;
   virtual ~Executor();

   Executor &operator=(Executor &&other);
   Executor &operator=(const Executor &other) = delete;

   /* Initializes internal Executor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Erases the passed Script. The reference becomes invalid after this is called.
   */
   void erase(Script *script);

   /* Adds an Script allocator with initialization information to this manager, allowing its given
      name to be used for future spawns.
      - allocator - Reference to instance of class implementing AllocatorInterface.
      - name - name to associate with the allocator
      - group - value to associate with all instances of this Script
      - removeonkill - removes this Script from this manager when it is killed
      - spawn_callback - function callback to call after Script has been spawned and setup
      - remove_callback - function callback to call before Script has been removed
   */
   void add(AllocatorInterface *allocator, const char *name, int group, bool removeonkill, std::function<void(Script*)> spawn_callback, std::function<void(Script*)> remove_callback);

   /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
   void enqueueSpawn(const char *script_name, int execution_queue, int tag);
   /* Enqueues a Script instance to be executed when runExecQueue() is called. */
   void enqueueExec(Script *script, unsigned queue);
   /* Enqueues a Script instance to be killed when runKillQueue() is called. */
   void enqueueKill(Script *script);

   /* Executes all currently enqueued Scripts in the specified queue, and dequeues them. This will call the 
      (init() method if it has not yet been called, and the) base() method on every active Script.
   */
   void runExecQueue(unsigned queue);
   /* Calls the kill() method on all erasure-queued Scripts if it has not been called yet. */
   void runKillQueue();
   /* Spawns all Scripts (or sub classes) queued for spawning with spawnScriptEnqueue(). */
   std::vector<Script*> runSpawnQueue();

   /* Returns true if the provided Script reference is owned by this instance. */
   bool has(Script *script);
   /* Returns true if the provided Script name has been previously added to this manager. */
   bool hasAdded(const char *script_name);
   /* Returns the number of Scripts in this executor. */
   unsigned getCount();
   /* Returns number of execution queues in this executor. */
   int getQueueCount();

   bool initialized();
};

// --------------------------------------------------------------------------------------------------------------------------

// prototype
template<typename T>
class Provider;

/* class Receiver
   Interface that is used to allow reception of a generic specified type when
   implemented, via subscription to a Provider of the same type.
*/
template<class T>
class Receiver {
    friend Provider<T>;

    // reference to provider
    Provider<T> *_r_provider;
    int _channel;
    bool _reception;

protected:
    // invoked on provider allocation if tag matches this receiver's channel
    virtual void _receive(T *t) {};
    Receiver() : _r_provider(nullptr), _channel(-1), _reception(false) {}

public:
    virtual ~Receiver() {
        unsubscribeFromProvider();
    }

    void setChannel(int channel) { _channel = channel; }
    void enableReception(bool state) { _reception = state; }
    int getChannel() { return _channel; }
    
    /* Returns reference to provider's active set of T references, if it exists. */
    const std::unordered_set<T*>* getAllProvided() { 
        if (_r_provider)
            return _r_provider->getAllProvided(); 
        else
            return nullptr;
    }

    /* Unsubscribes from subscribed provider. Does nothing if not subscribed. */
    void unsubscribeFromProvider() {
        if (_r_provider)
            _r_provider->tryUnsubscribe(this);
    }
};

/* class ProvidedType
   Interface that is used to allow storage and retrieval of the inheriting
   class by a Provider. Classes that do not implement this interface cannot be
   allocated by Providers. Template parameter type must be the inheriting class.
*/
template<class T>
class ProvidedType {
   friend Provider<T>;

   Provider<T> *_pt_provider;
   T *_t_ref;

protected:
   ProvidedType() : _pt_provider(nullptr), _t_ref(nullptr) {}

public:
   virtual ~ProvidedType() {
      removeFromProvider();
   }

   /* Removes from containing provider. Does nothing if not contained within a provider. */
   void removeFromProvider() {
      if (_pt_provider)
         _pt_provider->tryRemoveProvidedType(_t_ref);
   }
};

/* abstract class ProvidedAllocator
   Interface that extends AllocatorInterface to have its allocations intercepted and stored
   by a containing Provider. Only classes inheriting ProvidedType<T> can be instantiated
   by this class.
*/
template<class T>
class ProvidedAllocator : public AllocatorInterface {
   friend Provider<T>;

   Provider<T> *_a_provider;
   std::string _name;
   
   Script *_allocate(int tag) override {
      return _allocateStore(tag);
   }

protected:
   T * _allocateStore(int tag) {
      T *t = _allocateProvided();

      // if in a provider, give it this T
      if (_a_provider)
            _a_provider->_storeType(t, tag);
      
      return t;
   }

   virtual T *_allocateProvided() = 0;

   ProvidedAllocator() : _a_provider(nullptr) {}

public:
   virtual ~ProvidedAllocator() {
      removeFromProvider();
   }

   /* Removes from containing provider. Does nothing if not contained within a provider. */
   void removeFromProvider() {
      if (_a_provider)
         _a_provider->tryRemoveAllocator(_name);
   }
};

/* class Provider
   Stores ProvidedAllocators of the same templated type. Whenever its stored allocators are
   invoked, the instance is stored here for broadcasting and getting by any subscribed
   Receivers.
*/
template<class T>
class Provider {
   friend ProvidedAllocator<T>;

   std::unordered_set<Receiver<T>*> _receivers;
   std::unordered_set<T*> _providedtypes;
   std::unordered_map<std::string, ProvidedAllocator<T>*> _allocators;

protected:
   // stores and broadcasts instances of T
   void _storeType(T *t, int tag) {
      // interpret tag as channel

      // set fields of providedtype and store it
      _providedtypes.insert(t);
      t->_pt_provider = this;
      t->_t_ref = t;

      // if channel is non-negative, deliver instance
      if (tag >= 0) {
         // if channel matches, deliver
         for (const auto& receiver: _receivers)
               if (receiver->_reception)
                  if (receiver->_channel == tag)
                           receiver->_receive(t);
      }
   }

public:
   virtual ~Provider() {
      for (const auto& receiver: _receivers)
         receiver->_r_provider = nullptr;
      for (const auto& providedtype: _providedtypes)
         providedtype->_pt_provider = nullptr;
      for (const auto& allocator: _allocators)
         allocator.second->_a_provider = nullptr;
   }

   /* Adds the allocator to this provider's set of allocators, enabling interception of their allocations. */
   void addAllocator(ProvidedAllocator<T> *allocator, const char *name) {
      if (allocator->_a_provider)
         throw std::runtime_error("Attempt to add already added ProvidedAllocator");
      _allocators[name] = allocator;
      allocator->_a_provider = this;
      allocator->_name = name;
   }

   /* Subscribes the receiver to this provider's allocations, enabling receiving and getting instances. */
   void subscribe(Receiver<T> *receiver) {
      if (receiver->_r_provider)
         throw std::runtime_error("Attempt to subscribe already subscribed Receiver");
      _receivers.insert(receiver);
      receiver->_r_provider = this;
   }

   /* Tries to unsubscribe the receiver from this provider's allocations. Does nothing if not subscribed. */
   void tryUnsubscribe(Receiver<T> *r) {
      if (_receivers.find(r) != _receivers.end())
         _receivers.erase(r);
   }

   /* Tries to remove the T reference from this provider's storage. Does nothing if not contained. */
   void tryRemoveProvidedType(T *t) {
      if (_providedtypes.find(t) != _providedtypes.end())
         _providedtypes.erase(t);
   }

   /* Tries to remove a named allocator from this provider's storage. Does nothing if the name is not contained. */
   void tryRemoveAllocator(std::string name) {
      if (_allocators.find(name) != _allocators.end())
         _allocators.erase(name);
   }

   /* Returns read-only reference to all stored T references in this provider. */
   const std::unordered_set<T*>* getAllProvided() {
      return &_providedtypes;
   }

   /* Returns the number of ProvidedType instances currently contained. */
   unsigned getProvidedCount() {
      return _providedtypes.size();
   }

   /* Returns the allocator mapped to the name. */
   ProvidedAllocator<T>* getAllocator(const char *name) {
      return _allocators[name];
   }
};

#endif