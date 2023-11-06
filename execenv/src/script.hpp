#pragma once

#ifndef SCRIPT_HPP_
#define SCRIPT_HPP_

// prototype
class ExecEnv;

/* Class to represent a runnable script by an owning ExecEnv instance.
   The owning ExecEnv will call init(), base(), and kill() as needed, and
   excepts _init(), _base(), and _kill() to be implemented by children. 
*/
class Script {
    // allow ExecEnv to access private fields
    friend class ExecEnv;

    // ExecEnv instance that owns this Script
    ExecEnv *_owner;
    // flags of whether this script has been initialized or killed, maintained by ExecEnv
    bool _initialized;
    bool _killed;
    // id of script maintained by ExecEnv
    int _id;

protected:
    /* Functions to be overridden by children.
       - _init() is called by init(). init() is called on execution, only for the first time the script is queued.
       - _base() is called by base(). base() is called on execution, each time the script is queued.
       - _kill() is called by kill(). kill() is called on erasure.
    */
    virtual void _init();
    virtual void _base();
    virtual void _kill();
    
public:
    Script();
    
    /* Functions wrapping the virtual versions of the same method, which are directly called by the ExecEnv.
       - init() is called on execution, only for the first time the script is queued.
       - base() is called on execution, each time the script is queued.
       - kill() is called on erasure.
    */
    void init();
    void base();
    void kill();

    /* Returns the ID of the Script given by its owning ExecEnv. */
    int id();
    ExecEnv *owner();
};

#endif