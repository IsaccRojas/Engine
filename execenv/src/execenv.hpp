#pragma once

#ifndef EXECENV_HPP_
#define EXECENV_HPP_

#include <iostream>
#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <glm\glm.hpp>

#include "util.hpp"
#include "script.hpp"

/* Class to encapsulate execution environment for queue-able, inheritable Script instances.
   Uses queues to control execution and erasure of Script instances, and allows the user to
   define allocators of Scripts which can be mapped to strings.

*/
class ExecEnv {
    /* Script data structures */
    // IDs to distribute to Scripts
    Partitioner _ids;
    // vector of unique_ptrs of Scripts
    std::vector<Script*> _scripts;

    /* Execution data structures */
    // queue of IDs to be executed
    std::queue<int> _execqueue;
    // queue of IDS to be erased
    std::queue<int> _erasequeue;
    
    /* environment system variables */
    // maximum number of active scripts allowed
    int _maxcount;

    /* Erases the provided ID from the system by making its ID available for writing. 
       id - ID to be erased
    */
    void _erase(int id);

public:
    ExecEnv(int maxcount);
    ~ExecEnv();
    ExecEnv(const ExecEnv &other) = delete;
    ExecEnv operator=(const ExecEnv &other) = delete;
    
    /* Pushes a Script instance to the environment. This will generate an ID that will be used by the environment for
       checks and execution. This ID will be valid for the lifetime of the Script instance within the system. 
       script - pointer to an instance of Script
       Returns the ID of the Script.
    */
    int push(Script *script);
    
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
    /* Queues all Script instances in environment. */
    void queueExecAll();
    /* Pushes a Script instance into the erasure queue of the environment. Upon calling exec_erasequeue(), this ID will be
       made available for use with push(). This will not immediately deallocate the Script memory. Note that it is undefined 
       behavior to use the passed ID after calling this.
       id - ID of Script to be erased 
    */
    void queueErase(int id);

    /* Executes all currently queued Scripts, and dequeues them. This will call the (init() method if it has not yet been
       called, and the) base() method on every active Script. */
    void runExec();
    /* Calls the kill() method on all erasure-queued Scripts if it has not been called yet, and erases the Script from the
       environment, making all erased IDs available. */
    void runErase();
};

#endif