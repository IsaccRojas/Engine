#pragma once

#ifndef EXECUTOR_HPP_
#define EXECUTOR_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <glm\glm.hpp>

#include "util.hpp"

// prototype
class Executor;
class ManagerPtr;

/* Class to represent a runnable script by an owning Executor instance.
   The owning Executor will call init(), base(), and kill() as needed, and
   expects _init(), _base(), and _kill() to be implemented by children. 
*/
class Script {
    // allow Executor to access private fields
    friend class Executor;
    friend class Manager;

    // flags of whether this script has been initialized or killed, maintained by Executor
    bool _initialized;
    bool _killed;
    bool _execqueued;
    bool _killqueued;

    // environmental references
    Executor *_executor;   
    int _executor_id;
    bool _executor_ready;

    // Manager instance that owns this Script, and internal name
    ManagerPtr *_manager;

    // flag of whether or not this script has a manager or not
    bool _hasmanager;

protected:
    /* Functions to be overridden by children.
       - _init() is called by _runInit(). _runInit() is called on execution, only for the first time the script is queued.
       - _base() is called by _runBase(). _runBase() is called on execution, each time the script is queued.
       - _kill() is called by _runKill(). _runKill() is called on erasure.
    */
    virtual void _init();
    virtual void _base();
    virtual void _kill();

    /* Functions wrapping the virtual versions of the same method, which are directly called by the Executor.
       - _runInit() is called on execution, only for the first time the script is queued.
       - _runBase() is called on execution, each time the script is queued.
       - _runKill() is called on erasure.
    */
    void _runInit();
    void _runBase();
    void _runKill();

public:
    Script();
    ~Script();

    /* Sets the object up with an executor. This enables the use of 
       the enqueue and dequeue methods. Pushes the object into the
       executor.
    */
    void scriptSetup(Executor *executor);

    /* Enqueues the script for execution.
    */
    void enqueue();

    /* Kills the script.
    */
    void kill();
    
    ManagerPtr *getManager();
    bool hasManager(); 
};

/* Class to encapsulate execution environment for queue-able, inheritable Script instances.
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
    // maximum number of active scripts allowed
    int _maxcount;

public:
    Executor(int maxcount);
    ~Executor();
    Executor(const Executor &other) = delete;
    Executor operator=(const Executor &other) = delete;
    
    /* Pushes a Script instance to the environment. This will generate an ID that will be used by the environment for
       checks and execution. This ID will be valid for the lifetime of the Script instance within the system. 
       script - pointer to an instance of Script
       Returns the ID of the Script.
    */
    int push(Script *script);

    /* Erases the provided ID from the system by making its ID available for writing. Note that it is undefined 
       behavior to use the passed ID after calling this.
       id - ID to be erased
    */
    void erase(int id);
    
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

#endif