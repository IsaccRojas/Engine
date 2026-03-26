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
class Script;
class Executor;
class ScriptAllocatorInterface;

/* class ScriptKey
   Used to lock-out a Script, determining whether it can be removed or not. Held by Scripts.
*/
class ScriptKey {
   friend Script;
   ScriptKey();
};

/* class Script
   Represents a runnable script by an owning Executor instance.
   The owning Executor will call runInit(), runBase(), and runKill() as needed, and
   expects _init(), _base(), and _kill() to be implemented by children. 
*/
class Script {
   friend Executor;
   friend ScriptAllocatorInterface;

   // fields maintained by owning Executor
   Executor* _executor;
   ScriptAllocatorInterface* _scriptallocator;
   std::list<Script*>::iterator _this_iter;
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
      - _exec() is called by runExec(). runExec() is called on execution, each time the Script is queued.
      - _kill() is called by runKill(). runKill() is called on erasure.
      - _update() is called by runUpdate(). runUpdate() is called when update() is called by the owning Executor.
   */
   virtual void _init() = 0;
   virtual void _exec() = 0;
   virtual void _kill() = 0;
   virtual void _update() = 0;

public:
   Script(Script&& other);
   Script();
   Script(const Script&) = delete;
   virtual ~Script();

   Script& operator=(Script&& other);
   Script& operator=(const Script&) = delete;

/* Functions wrapping the virtual versions of the same method, which are directly called by the Executor.
      - runInit() is called on spawn.
      - runExec() is called on execution, each time the Script is queued.
      - runKill() is called on erasure.
      - runUpdate() is called when update() is called by the owning Executor.
   */
   void runInit();
   void runExec();
   void runKill();
   void runUpdate();

   /* Enqueues the Script for execution.
      - queue - queue to enqueue into
   */
   void enqueueExec(unsigned queue);

   /* Kills the Script. */
   void enqueueKill();

   /* Gets various internal flags used by Executors to control state.
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
   Contains a Script reference and wraps access to Script data without owning it. Invalid if the viewed Script is destroyed.
*/
class ScriptView {
   friend Executor;
   Script* _script;
public:
   ScriptView(Script* script);

   void enqueueExec(unsigned queue);
   void enqueueKill();
   int getLastExecQueue();
   bool getExecEnqueued();
   bool getKillEnqueued();
   const char* getName();
   ScriptKey key();
   void lockout(ScriptKey* k);
   void unlock(ScriptKey* k);
   Script* getScript();
};

// --------------------------------------------------------------------------------------------------------------------------

class Executor;

/* abstract class ScriptAllocatorInterface
   Is used to invoke _allocate(), which must return heap-allocated memory to be owned
   by the invoking EntityExecutor instance.

   Stores a reference that can be checked against for existence by subtypes.
*/
class ScriptAllocatorInterface {
   friend Script;
   friend Executor;
   
   std::unordered_set<Script*> _scripts;

   // inserts reference into set (does not allocate memory)
   void _insertReference(Script* script);

   // removes reference from set (does not delete memory)
   void _removeReference(Script* script);

protected:
   // must return a heap-allocated instance of a covariant type of EntityScript
   virtual Script* _allocate() = 0;

public:
   virtual ~ScriptAllocatorInterface();

   // checks if reference came from this allocator
   bool hasReference(Script* script);
};

/* class ScriptProvider<T>
   Templated implementation of the ScriptAllocatorInterface, that can provide subtype references
   of allocated Script types.
*/
template<class T>
class ScriptProviderInterface : public ScriptAllocatorInterface {
   std::unordered_map<Script*, T*> _Ts;

   Script* _allocate() override {
      T* t = new T;
      _Ts[t] = t;
      return t;
   }

protected:
   virtual T* _providerAllocate() = 0;

public:
   T* getInstance(Script* script) {
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

/* class Executor
   Encapsulates an execution environment for queue-able, inheritable Script instances.
   Uses queues to control execution and erasure of Scripts. A variable number of queues 
   for execution can be specified.
*/
class Executor {
protected:
   // class to store enqueues and polymorphically spawn later
   class ScriptEnqueue {
      friend Executor;
      Executor* _executor;
   protected:
      std::string _name;
      int _execution_queue;
      virtual ScriptView spawn();
      ScriptEnqueue(Executor *executor, std::string name, int execution_queue);
      // default copy assignment/construction are fine (copying implies another enqueue in the same Executor)
   public:
      virtual ~ScriptEnqueue();
   };

private:
   /* Script data structures */
   // memory-managed list of Script references
   ManagedList<Script> _scripts;

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
   void _setupScript(Script* script, const char* script_name, int execution_queue, ScriptAllocatorInterface* scriptallocator);

   // pushes an enqueue
   void _pushSpawnEnqueue(ScriptEnqueue* enqueue);

   // erases the passed Script; it is undefined behavior to use the ScriptView after this call
   void _erase(Script* script);

public:
   /* Calls init() with the provided arguments. */
   Executor(unsigned queues);
   Executor();
   Executor(Executor&& other);
   Executor(const Executor& other) = delete;
   virtual ~Executor();

   Executor& operator=(Executor&& other);
   Executor& operator=(const Executor& other) = delete;

   /* Initializes internal Executor data. It is undefined behavior to make calls on this instance
      before calling this and after uninit().
   */
   void init(unsigned queues);
   void uninit();

   /* Stores ScriptInfo with allocator and initialization information in this Executor, mapped to the provided name.
      - ScriptInfo - instance of ScriptInfo with allocation/initialization information
      - name - name to associate with the ScriptInfo instance
   */
   void addScript(ScriptInfo scriptinfo, const char* name);

   /* Spawns a Script using a name previously added to this Executor, calls its runInit() method, and returns a ScriptView of it. */
   ScriptView spawnScript(const char* script_name, int execution_queue);

   /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
   void enqueueSpawn(const char* script_name, int execution_queue);
   /* Enqueues a Script instance to be executed when runExecQueue() is called. */
   void enqueueExec(ScriptView scriptview, unsigned queue);
   /* Enqueues a Script instance to be killed and removed when runKillQueue() is called. */
   void enqueueKill(ScriptView scriptview);

   /* Spawns all Scripts (or sub classes) queued for spawning with spawnScriptEnqueue(). */
   std::vector<ScriptView> runSpawnQueue();
   /* Executes all currently enqueued Scripts in the specified queue, and dequeues them. This will call the 
      runExec() method on every active Script.
   */
   void runExecQueue(unsigned queue);
   /* Calls the runKill() method on all kill-queued Scripts if it has not been called yet, and erases the Scripts. */
   void runKillQueue();
   /* Calls runUpdate() method on all Scripts. */
   void runUpdate();

   /* Returns true if the provided Script name has been previously added to this Executor. */
   bool hasAdded(const char* script_name);
   /* Returns the number of Scripts in this Executor. */
   unsigned getCount();
   /* Returns number of execution queues in this Executor. */
   int getQueueCount();

   /* Returns whether or not this Executor instance has been initialized or not. */
   bool initialized();
};

// --------------------------------------------------------------------------------------------------------------------------

#endif