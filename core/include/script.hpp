#ifndef SCRIPT_HPP_
#define SCRIPT_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <unordered_set>

#include "commonexcept.hpp"
#include "managedlist.hpp"
#include "managedqueue.hpp"
#include "util.hpp"

// prototype
class ScriptInterface;
class ScriptExecutor;
class ScriptAllocatorInterface;

/* class ScriptKey
   Used to lock-out a ScriptInterface, determining whether it can be removed or not. Held by ScriptInterfaces.
*/
class ScriptKey {
   friend ScriptInterface;
   ScriptKey();
};

/* class ScriptInterface
   Represents a runnable script by an owning ScriptExecutor instance.
   The owning ScriptExecutor will call runInit(), runBase(), and runKill() as needed, and
   expects _init(), _base(), and _kill() to be implemented by children. 
*/
class ScriptInterface {
   friend ScriptExecutor;
   friend ScriptAllocatorInterface;

   // fields maintained by owning ScriptExecutor
   ScriptExecutor* _executor;
   ScriptAllocatorInterface* _scriptallocator;
   std::list<ScriptInterface*>::iterator _this_iter;
   int _last_execqueue;
   bool _exec_enqueued;
   bool _kill_enqueued; 
   std::string _script_name;

   // lockout variables
   ScriptKey _key;
   std::unordered_set<ScriptKey*> _keys;
   unsigned _keys_count;
   
protected:
   /* Functions to be overridden by children.
      - _init() is called by runInit(). runInit() is called on spawn.
      - _exec() is called by runExec(). runExec() is called on execution, each time the ScriptInterface is queued.
      - _kill() is called by runKill(). runKill() is called on erasure.
      - _update() is called by runUpdate(). runUpdate() is called when update() is called by the owning ScriptExecutor.
   */
   virtual void _init() = 0;
   virtual void _exec() = 0;
   virtual void _kill() = 0;
   virtual void _update() = 0;

public:
   ScriptInterface(ScriptInterface&& other);
   ScriptInterface();
   ScriptInterface(const ScriptInterface&) = delete;
   virtual ~ScriptInterface();

   ScriptInterface& operator=(ScriptInterface&& other);
   ScriptInterface& operator=(const ScriptInterface&) = delete;

/* Functions wrapping the virtual versions of the same method, which are directly called by the ScriptExecutor.
      - runInit() is called on spawn.
      - runExec() is called on execution, each time the ScriptInterface is queued.
      - runKill() is called on erasure.
      - runUpdate() is called when update() is called by the owning ScriptExecutor.
   */
   void runInit();
   void runExec();
   void runKill();
   void runUpdate();

   /* Enqueues the ScriptInterface for execution.
      - queue - queue to enqueue into
   */
   void enqueueExec(unsigned queue);

   /* Kills the ScriptInterface. */
   void enqueueKill();

   /* Gets various internal flags used by ScriptExecutors to control state.
   */
   int getLastExecQueue();
   bool getExecEnqueued();
   bool getKillEnqueued();
   const char* getName();
   ScriptKey& key();
   void lockout(ScriptKey* k);
   void unlock(ScriptKey* k);
   unsigned lockout_count();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ScriptView
   Contains a ScriptInterface reference and wraps access to ScriptInterface data without owning it. Invalid if the viewed ScriptInterface is destroyed.
*/
class ScriptView {
   friend ScriptExecutor;
   ScriptInterface* _script;
public:
   ScriptView(ScriptInterface* script);

   void enqueueExec(unsigned queue);
   void enqueueKill();
   int getLastExecQueue();
   bool getExecEnqueued();
   bool getKillEnqueued();
   const char* getName();
   ScriptKey key();
   void lockout(ScriptKey* k);
   void unlock(ScriptKey* k);
   ScriptInterface* getScript();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ScriptAllocatorInterface
   Is used to invoke _allocate(), which must return heap-allocated memory to be owned
   by the invoking ScriptExecutor instance.

   Stores a reference that can be checked against for existence by subtypes.
*/
class ScriptAllocatorInterface {
   friend ScriptInterface;
   friend ScriptExecutor;
   
   std::unordered_set<ScriptInterface*> _scripts;

   // inserts reference into set (does not allocate memory)
   void _insertReference(ScriptInterface* script);

   // removes reference from set (does not delete memory)
   void _removeReference(ScriptInterface* script);

protected:
   // must return a heap-allocated instance of a covariant type of ScriptInterface
   virtual ScriptInterface* _allocate() = 0;

public:
   virtual ~ScriptAllocatorInterface();

   // checks if reference came from this allocator
   bool hasReference(ScriptInterface* script);
};

/* class ScriptProvider<T>
   Templated implementation of the ScriptAllocatorInterface, that can provide subtype references
   of allocated ScriptInterface types.
*/
template<class T>
class ScriptProviderInterface : public ScriptAllocatorInterface {
   std::unordered_map<ScriptInterface*, T*> _Ts;

   ScriptInterface* _allocate() override {
      T* t = new T;
      _Ts[t] = t;
      return t;
   }

protected:
   virtual T* _providerAllocate() = 0;

public:
   T* getInstance(ScriptInterface* script) {
      if (!hasReference(script))
         throw std::runtime_error("Attempt to get subtype instance with script address that this allocator did not allocate");
      return _Ts[script];
   }
};

/* class GenericScriptProvider<T>
   Generic implementation of ScriptProvider<T>.
*/
template<class T>
class GenericScriptProvider : public ScriptProviderInterface<T> {
   T* _providerAllocate() override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

struct ScriptInfo {
   ScriptAllocatorInterface* _allocator;
   std::function<void(ScriptView)> _spawn_callback;
   std::function<void(ScriptView)> _remove_callback;
   // default copy assignment/construction are fine
};

/* class ScriptExecutor
   Encapsulates an execution environment for queue-able, inheritable ScriptInterface instances.
   Uses queues to control execution and erasure of ScriptInterfaces. A variable number of queues 
   for execution can be specified.
*/
class ScriptExecutor {
protected:
   // class to store enqueues and polymorphically spawn later
   class ScriptEnqueue {
      friend ScriptExecutor;
      ScriptExecutor* _executor;
   protected:
      std::string _name;
      int _execution_queue;
      virtual ScriptView spawn();
      ScriptEnqueue(ScriptExecutor *executor, std::string name, int execution_queue);
      // default copy assignment/construction are fine (copying implies another enqueue in the same ScriptExecutor)
   public:
      virtual ~ScriptEnqueue();
   };

private:
   /* ScriptInterface data structures */
   // memory-managed list of ScriptInterface references
   ManagedList<ScriptInterface> _scripts;

   // internal variables for added script information and active scripts
   std::unordered_map<std::string, ScriptInfo> _scriptinfos;
   ManagedQueue<ScriptEnqueue> _scriptenqueues;

   // queues of ScriptInterfaces to be executed; swapped on execution
   struct QueuePair {
      std::queue<ScriptInterface*> _push_execqueue;
      std::queue<ScriptInterface*> _run_execqueue;
   };
   std::vector<QueuePair> _queuepairs;
   
   // queue of ScriptInterfaces to be erased
   std::queue<ScriptInterface*> _push_killqueue;
   std::queue<ScriptInterface*> _run_killqueue;

   bool _initialized;

protected:
   // initializes ScriptInterface's ScriptExecutor-related fields
   void _setupScript(ScriptInterface* script, const char* script_name, int execution_queue, ScriptAllocatorInterface* scriptallocator);

   // pushes an enqueue
   void _pushSpawnEnqueue(ScriptEnqueue* enqueue);

   // erases the passed ScriptInterface; it is undefined behavior to use the ScriptView after this call
   void _erase(ScriptInterface* script);

public:
   /* Calls init() with the provided arguments. */
   ScriptExecutor(unsigned queues);
   ScriptExecutor();
   ScriptExecutor(ScriptExecutor&& other);
   ScriptExecutor(const ScriptExecutor& other) = delete;
   virtual ~ScriptExecutor();

   ScriptExecutor& operator=(ScriptExecutor&& other);
   ScriptExecutor& operator=(const ScriptExecutor& other) = delete;

   /* Initializes internal ScriptExecutor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Stores ScriptInfo with allocator and initialization information in this ScriptExecutor, mapped to the provided name.
      - ScriptInfo - instance of ScriptInfo with allocation/initialization information
      - name - name to associate with the ScriptInfo instance
   */
   void addScript(ScriptInfo scriptinfo, const char* name);

   /* Spawns a script using a name previously added to this ScriptExecutor, calls its runInit() method, and returns a ScriptView of it. */
   ScriptView spawnScript(const char* script_name, int execution_queue);

   /* Enqueues a script to be spawned when calling runSpawnQueue(). */
   void enqueueSpawn(const char* script_name, int execution_queue);
   /* Enqueues a script instance to be executed when runExecQueue() is called. */
   void enqueueExec(ScriptView scriptview, unsigned queue);
   /* Enqueues a script instance to be killed and removed when runKillQueue() is called. */
   void enqueueKill(ScriptView scriptview);

   /* Spawns all scripts (or sub classes) queued for spawning with spawnScriptEnqueue(). */
   std::vector<ScriptView> runSpawnQueue();
   /* Executes all currently enqueued scripts in the specified queue, and dequeues them. This will call the 
      runExec() method on every active script.
   */
   void runExecQueue(unsigned queue);
   /* Calls the runKill() method on all kill-queued scripts if it has not been called yet, and erases the scripts. */
   void runKillQueue();
   /* Calls runUpdate() method on all scripts. */
   void runUpdate();

   /* Returns true if the provided script name has been previously added to this ScriptExecutor. */
   bool hasAdded(const char* script_name);
   /* Returns the number of scripts in this ScriptExecutor. */
   unsigned getCount();
   /* Returns number of execution queues in this ScriptExecutor. */
   int getQueueCount();

   /* Returns whether or not this ScriptExecutor instance has been initialized or not. */
   bool initialized();
};

// --------------------------------------------------------------------------------------------------------------------------

#endif