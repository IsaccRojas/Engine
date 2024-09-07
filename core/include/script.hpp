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

    // flags of whether this Script has been initialized or killed, maintained by Executor
    bool _initialized;
    bool _killed;
    bool _exec_queued;
    bool _kill_queued;

    // environmental references
    Executor *_executor;   
    int _executor_id;

    // Manager instance that owns this Script, ID, and removeonkill flag; maintained by Manager
    ScriptManager *_scriptmanager;
    int _scriptmanager_id;
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
    void setInitialized(bool initialized);
    bool getInitialized();
    void setKilled(bool killed);
    bool getKilled();
    void setExecQueued(bool execqueued);
    bool getExecQueued();
    void setKillQueued(bool killqueued);
    bool getKillQueued();
    void setGroup(int group);
    int getGroup();

    /* Enqueues the Script for execution.
    */
    void enqueue();

    /* Kills the Script.
    */
    void kill();
    
    ScriptManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class Executor
   Encapsulates an execution environment for queue-able, inheritable Script instances.
   Uses queues to control execution and erasure of Script instances.

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
    // queues of IDs to be executed; swapped on execution
    std::queue<unsigned> _push_execqueue;
    std::queue<unsigned> _run_execqueue;
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
    Executor(unsigned max_count);
    Executor(Executor &&other);
    Executor();
    Executor(const Executor &other) = delete;
    ~Executor();

    Executor &operator=(Executor &&other);
    Executor &operator=(const Executor &other) = delete;

    void init(unsigned max_count);
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

    /* Queues a Script instance corresponding to the ID provided to be executed when exec() is called.
       id - ID of Script to queue
    */
    void queueExec(unsigned id);

    /* Pushes a Script instance into the kill queue of the executor.
       id - ID of Script to be erased 
    */
    void queueKill(unsigned id);

    /* Executes all currently queued Scripts, and dequeues them. This will call the (init() method if it has not yet been
       called, and the) base() method on every active Script. */
    void runExec();
    /* Calls the kill() method on all erasure-queued Scripts if it has not been called yet. */
    void runKill();

    /* Returns true if the provided ID is active. */
    bool hasID(unsigned id);
    /* Returns whether this instance has been initialized or not. */
    bool getInitialized();
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
       bool _force_enqueue;
       bool _force_removeonkill;
       std::function<Script*(void)> _allocator = nullptr;
       std::function<void(Script*)> _spawn_callback = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed script during its lifetime
    struct ScriptValues {
       unsigned _manager_id;
       const char *_manager_name;
       Script *_script_ref;
       int _group;
    };

    // _valid flag is used to prevent instance from being spawned as a Script
    struct ScriptQueue {
       bool _valid;
       std::string _name;
    };

protected:
    // internal variables for added script information and active scripts
    std::unordered_map<std::string, ScriptInfo> _scriptinfos;
    std::vector<ScriptValues> _scriptvalues;
    std::queue<ScriptQueue> _scriptqueues;

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
    void _scriptSetup(Script *script, ScriptInfo &info, unsigned id);
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
       will invoke scriptSetup() if set to do so from adding it.
    */
    virtual unsigned spawnScript(const char *script_name);
    /* Queues a Script to be spawned when calling runSpawnQueue(). */
    virtual void spawnScriptQueue(const char *script_name);
    /* Spawns all Scripts (or sub classes) queued for spawning with spawnScriptQueue(). */
    virtual std::vector<unsigned> runSpawnQueue();
    /* Removes the Script associated with the provided ID. */
    void remove(unsigned id);

    /* Adds an Script allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - function pointer referring to function that returns a heap-allocated Script
       - name - name to associate with the allocator
       - group - value to associate with all instances of this Script
       - force_enqueue - enqueues this Script into the provided Executor when spawning it
       - force_removeonkill - removes this Script from this manager when it is killed
       - spawn_callback - function callback to call after Script has been spawned and setup
    */
    void addScript(std::function<Script*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, std::function<void(Script*)> spawn_callback);

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