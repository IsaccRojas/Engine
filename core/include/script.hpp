#ifndef SCRIPT_HPP_
#define SCRIPT_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <glm\glm.hpp>

#include "util.hpp"

// prototype
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
    bool _execqueued;
    bool _killqueued;

    // environmental references
    Executor *_executor;   
    int _executor_id;
    bool _executor_ready;

    // Manager instance that owns this Script, ID, and removeonkill flag; maintained by Manager
    ScriptManager *_scriptmanager;
    int _scriptmanager_id;
    bool _scriptmanager_ready;
    bool _scriptmanager_removeonkill;

    // settable integer usable for identification
    int _type;

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
    Script();
    virtual ~Script();

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
    
    /* Resets executor information.
    */
    void scriptResetExec();

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
    void setType(int type);
    int getType();

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
*/
class Executor {
    /* Script data structures */
    // IDs to distribute to Scripts
    Partitioner _ids;
    // vector of unique_ptrs of Scripts
    std::vector<Script*> _scripts;

    /* Execution data structures */
    // queues of IDs to be executed; swapped on execution
    std::queue<int> _pushexecqueue;
    std::queue<int> _runexecqueue;
    // queue of IDS to be erased
    std::queue<int> _pushkillqueue;
    std::queue<int> _runkillqueue;
    
    /* environment system variables */
    // maximum number of active Scripts allowed
    unsigned _maxcount;

public:
    Executor(unsigned maxcount);
    ~Executor();
    Executor(const Executor &other) = delete;
    Executor operator=(const Executor &other) = delete;
    
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
    void remove(int id);
    
    /* Returns a pointer to a Script instance in the environment corresponding to the provided ID. Returns nullptr if the
       ID does not exist in the environment.
       id - ID of Script to get pointer of
    */
    Script* const get(int id);

    /* Returns true if the provided ID is valid; false otherwise.
       id - ID of Script to check validity of
    */
    bool has(int id);

    /* Queues a Script instance corresponding to the ID provided to be executed when exec() is called.
       id - ID of Script to queue
    */
    void queueExec(int id);

    /* Pushes a Script instance into the kill queue of the executor.
       id - ID of Script to be erased 
    */
    void queueKill(int id);

    /* Executes all currently queued Scripts, and dequeues them. This will call the (init() method if it has not yet been
       called, and the) base() method on every active Script. */
    void runExec();
    /* Calls the kill() method on all erasure-queued Scripts if it has not been called yet. */
    void runKill();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ScriptManager
   Manages and controls internal instances of Scripts. Can also be given an 
   Executor instance to automatically pass Script instances to these mechanisms.
*/
class ScriptManager {
public:
    // struct holding script information mapped to a name
    struct ScriptType {
        int _internal_type;
        bool _force_scriptsetup;
        bool _force_enqueue;
        bool _force_removeonkill;
        std::function<Script*(void)> _allocator = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed script during its lifetime
    struct ScriptValues {
        int _manager_id;
        const char *_manager_name;
        Script *_script_ref;
    };

protected:
    // internal variables for added script information and active scripts
    std::unordered_map<std::string, ScriptType> _scripttypes;
    std::vector<ScriptValues> _scriptvalues;
    
    // "managed" structures
    Executor *_executor;

    // IDs to distribute to Scripts
    Partitioner _ids;
    // vector of unique_ptrs of Scripts
    std::vector<std::unique_ptr<Script>> _scripts;
    
    unsigned _maxcount;

    // internal methods called when spawning Scripts and removing them, using and setting
    // manager lifetime and Script runtime members
    void _scriptSetup(Script *script, ScriptType &type, int id);
    void _scriptRemoval(ScriptValues &values);
   
public:
    /* maxcount - maximum number of Scripts to support in this instance */
    ScriptManager(unsigned maxcount);
    virtual ~ScriptManager();

    /* Returns true if the provided Script name has been previously added to this manager. */
    bool hasScript(const char *scriptname);
    /* Returns a reference to the spawned Script corresponding to the provided ID, if it exists. */
    Script *getScript(int id);
    /* Returns the internal name corresponding to the provided ID, if it exists. */
    std::string getName(int id);

    /* Spawns a Script using a name previously added to this manager, and returns its ID. This
       will invoke scriptSetup() if set to do so from adding it.
    */
    virtual int spawnScript(const char *scriptname);
    
    /* Adds an Script allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - function pointer referring to function that returns a heap-allocated Script
       - name - name to associate with the allocator
       - type - internal value tied to Script for client use
       - force_scriptsetup - invokes Script setup when spawning this Script
       - force_enqueue - enqueues this Script into the provided Executor when spawning it
       - force_removeonkill - removes this Script from this manager when it is killed
    */
    void addScript(std::function<Script*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill);
    /* Removes the Script associated with the provided ID. */
    void remove(int id);

    /* Sets the Executor for this manager to use. */
    void setExecutor(Executor *executor);
    
    /* Gets the maximum generated ID during this manager's lifetime. */
    int getMaxID();
};

#endif