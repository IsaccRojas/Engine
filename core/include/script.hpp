#ifndef SCRIPT_HPP_
#define SCRIPT_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <glm\glm.hpp>

#include "util.hpp"
#include "commonexcept.hpp"

// prototypes
class Executor;
class ScriptManager;

/* class Script
   Represents a runnable script by an owning Executor instance.
   The owning Executor will call runInit(), runBase(), and runKill() as needed, and
   expects _init(), _base(), and _kill() to be implemented by children. 
*/
class Script {
    friend ScriptManager;
    friend Executor;

    // flags of whether this Script has been initialized or killed, maintained by Executor
    int _last_execqueue;
    bool _initialized;
    bool _killed;
    bool _exec_enqueued;
    bool _kill_enqueued;

    // environmental references
    Executor *_executor;   
    int _executor_id;

    // Manager instance that owns this Script, ID, and removeonkill flag; maintained by Manager
    ScriptManager *_scriptmanager;
    unsigned _scriptmanager_id;
    bool _scriptmanager_removeonkill;

    // settable integer usable for identification
    int _group;

protected:
    /* Functions to be overridden by children.
       - _init() is called by runInit(). _runInit() is called on execution, only for the first time the Script is queued.
       - _base() is called by runBase(). _runBase() is called on execution, each time the Script is queued.
       - _kill() is called by runKill(). _runKill() is called on erasure.
    */
    virtual void _init();
    virtual void _base();
    virtual void _kill();

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

    /* Sets the Script up with an Executor. This enables the use of 
       the enqueue and dequeue methods. Pushes the Script into the
       Executor.
    */
    void scriptSetup(Executor *executor);

    /* Resets internal execution flags.
    */
    void scriptResetFlags();
    
    /* Removes self from stored executor reference.
    */
    void scriptClear();

    /* Sets various internal flags used by Executors to control state. Can be set manually to manipulate
       execution behavior.
    */
    
    int getLastExecQueue();
    bool getInitialized();
    bool getKilled();
    bool getExecEnqueued();
    bool getKillEnqueued();
    int getGroup();

    /* Enqueues the Script for execution.
       - queue - queue to enqueue into
    */
    void enqueueExec(unsigned queue);

    /* Kills the Script.
    */
    void enqueueKill();
    
    unsigned getManagerID();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class Executor
   Encapsulates an execution environment for queue-able, inheritable Script instances.
   Uses queues to control execution and erasure of Script instances. A variable number
   of queues for execution can be specified.

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class Executor {
    /* Script data structures */
    // IDs to distribute to Scripts
    SlotVec _ids;
    // vector of unique_ptrs of Scripts
    std::vector<Script*> _scripts;

    /* Execution data structures */

    struct QueuePair {
      // queues of IDs to be executed; swapped on execution
      std::queue<unsigned> _push_execqueue;
      std::queue<unsigned> _run_execqueue;
    };
    std::vector<QueuePair> _queuepairs;
    
    // queue of IDS to be erased
    std::queue<unsigned> _push_killqueue;
    std::queue<unsigned> _run_killqueue;
    
    /* environment system variables */
    // maximum number of active Scripts allowed
    unsigned _max_count;
    unsigned _count;

    bool _initialized;
public:
    /* Calls init() with the provided arguments. */
    Executor(unsigned max_count, unsigned queues);
    Executor(Executor &&other);
    Executor();
    Executor(const Executor &other) = delete;
    ~Executor();

    Executor &operator=(Executor &&other);
    Executor &operator=(const Executor &other) = delete;

    void init(unsigned max_count, unsigned queues);
    void uninit();
    
    /* Pushes a Script instance to the environment. This will generate an ID that will be used by the environment for
       checks and execution. This ID will be valid for the lifetime of the Script instance within the system. 
       script - pointer to an instance of Script
       Returns the ID of the Script.
    */
    int push(Script *script);

    /* Removes the provided ID from the system by making its ID available for writing. Note that it is undefined 
       behavior to use the passed ID after calling this.
       id - ID to be removed
    */
    void remove(unsigned id);
    
    /* Returns a pointer to a Script instance in the environment corresponding to the provided ID. Returns nullptr if the
       ID does not exist in the environment.
       id - ID of Script to get pointer of
    */
    Script* const get(unsigned id);

    /* Enqueues a Script instance corresponding to the ID provided to be executed when runExecQueue() is called.
       id - ID of Script to enqueue
       queue - which internal queue to push the script into
    */
    void enqueueExec(unsigned id, unsigned queue);

    /* Enqueues a Script instance corresponding to the ID provided to be killed when runKillQueue() is called
       id - ID of Script to be erased 
    */
    void enqueueKill(unsigned id);

    /* Executes all currently enqueued Scripts, and dequeues them. This will call the (init() method if it has not yet been
       called, and the) base() method on every active Script. 
       - queue - which internal queue to run
    */
    void runExecQueue(unsigned queue);
    /* Calls the kill() method on all erasure-queued Scripts if it has not been called yet. */
    void runKillQueue();

    /* Returns true if the provided ID is active. */
    bool hasID(unsigned id);
    /* Returns whether this instance has been initialized or not. */
    bool getInitialized();
    /* Returns number of execution queues in this instance. */
    int getQueueCount();
};

// --------------------------------------------------------------------------------------------------------------------------

/* abstract class ScriptAllocatorInterface
   Is used to invoke allocate(), which must return heap-allocated memory to be owned
   by the invoking Manager instance.
*/
class ScriptAllocatorInterface {
public:
   /* Must return a heap-allocated instance of a covariant type of Script. */
   virtual Script *allocate(void) = 0;
};

/* class GenericScriptAllocator
   A generic implementation of the ScriptAllocatorInterface, that can be used if no
   special behavior or state is needed.
*/
template<class T>
class GenericScriptAllocator : public ScriptAllocatorInterface {
public:
    Script *allocate() override { return new T; }
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ScriptManager
   Manages and controls internal instances of Scripts. Can also be given an 
   Executor instance to automatically pass Script instances to these mechanisms.

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class ScriptManager {
public:
    // struct holding script information mapped to a name
    struct ScriptInfo {
       int _group;
       bool _force_removeonkill;
       ScriptAllocatorInterface *_allocator;
       std::function<void(unsigned)> _spawn_callback;
       std::function<void(unsigned)> _remove_callback;
    };

    // struct holding IDs and other flags belonging to the managed script during its lifetime
    struct ScriptValues {
       unsigned _manager_id;
       const char *_manager_name;
       Script *_script_ref;
       int _group;
    };

    // _valid flag is used to prevent instance from being spawned as a Script
    struct ScriptEnqueue {
       bool _valid;
       std::string _name;
       int _executor_queue;
    };

protected:
    // internal variables for added script information and active scripts
    std::unordered_map<std::string, ScriptInfo> _scriptinfos;
    std::vector<ScriptValues> _scriptvalues;
    std::queue<ScriptEnqueue> _scriptenqueues;

    // "managed" structures
    Executor *_executor;

    // IDs to distribute to Scripts
    SlotVec _ids;

    // vector of unique_ptrs of Scripts; allocated references are stored here so that they are
    // guaranteed to be deleted at some point, even if this vector remains unused
    std::vector<std::unique_ptr<Script>> _scripts;
    
    unsigned _max_count;
    unsigned _count;

    bool _initialized;

    // internal methods called when spawning Scripts and removing them, using and setting
    // manager lifetime and Script runtime members
    void _scriptSetup(Script *script, ScriptInfo &info, unsigned id, int executor_queue);
    void _scriptRemoval(ScriptValues &values);
   
public:
    /* Calls init() with the provided arguments. */
    ScriptManager(unsigned max_count, Executor *executor);
    ScriptManager(ScriptManager &&other);
    ScriptManager(const ScriptManager &other) = delete;
    ScriptManager();
    virtual ~ScriptManager();

    ScriptManager &operator=(ScriptManager &&other);
    ScriptManager &operator=(const ScriptManager &other) = delete;
    
    void init(unsigned max_count, Executor *executor);
    void uninit();

    /* Spawns a Script using a name previously added to this manager, and returns its ID. This
       will invoke scriptSetup().
    */
    virtual unsigned spawnScript(const char *script_name, int executor_queue);
    /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
    virtual void spawnScriptEnqueue(const char *script_name, int executor_queue);
    /* Spawns all Scripts (or sub classes) queued for spawning with spawnScriptEnqueue(). */
    virtual std::vector<unsigned> runSpawnQueue();
    /* Removes the Script associated with the provided ID. */
    virtual void remove(unsigned id);

    /* Adds an Script allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - Reference to instance of class implementing ScriptAllocatorInterface.
       - name - name to associate with the allocator
       - group - value to associate with all instances of this Script
       - force_removeonkill - removes this Script from this manager when it is killed
       - spawn_callback - function callback to call after Script has been spawned and setup
       - remove_callback - function callback to call before Script has been removed
    */
    void addScript(ScriptAllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback);

    /* Returns true if the provided Script name has been previously added to this manager. */
    bool hasAddedScript(const char *script_name);
    /* Returns true if the provided ID is active. */
    bool hasID(unsigned id);
    /* Returns a reference to the spawned Script corresponding to the provided ID, if it exists. */
    Script *getScript(unsigned id);
    /* Returns a vector of IDs of all active Scripts with the corresponding group. */
    std::vector<unsigned> getAllByGroup(int group);
    /* Returns the internal name corresponding to the provided ID, if it exists. */
    std::string getName(unsigned id);
    /* Returns the number of active objects in the manager. */
    unsigned getCount();
    /* Gets the maximum generated ID during this manager's lifetime. */
    int getMaxID();
    /* Returns whether this instance has been initialized or not. */
    bool getInitialized();
};

#endif