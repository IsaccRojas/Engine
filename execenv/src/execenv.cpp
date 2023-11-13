#include "execenv.hpp"

ExecEnv::ExecEnv(int maxcount) : 
    _maxcount(maxcount),
    _scripts(maxcount, nullptr)
{}

ExecEnv::~ExecEnv() {}

int ExecEnv::push(Script *script) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillsize() >= _maxcount)
        return -1;
    
    // get a new unique ID
    int id = _ids.push();

    // allocate new script
    _scripts[id] = script;
    
    // initialize owner, ID and flags
    _scripts[id]->_owner = this;
    _scripts[id]->_id = id;
    _scripts[id]->_initialized = false;
    _scripts[id]->_killed = false;

    return id;
}

void ExecEnv::_erase(int id) {
    // free ID from partitioner
    _ids.erase_at(id);
}

Script* const ExecEnv::get(int id) {
    if (_ids.at(id))
        // return "view" of stored Script
        return _scripts[id];
    else
        return nullptr;
}

bool ExecEnv::has(int id) {
    return _ids.at(id);
}

void ExecEnv::queueExec(int id) {
    _execqueue.push(id);
}

void ExecEnv::queueExecAll() {
    // queue all scripts for execution
    for (unsigned i = 0; i < _ids.size(); i++)
        if (_ids.at(i))
            _execqueue.push(i);
}

void ExecEnv::queueErase(int id) {
    // put script into erase queue
    _erasequeue.push(id);
}

void ExecEnv::runExec() {
    unsigned id;
    while (!(_execqueue.empty())) {
        id = _execqueue.front();

        // check if ID is active
        if (_ids.at(id)) {
            // check if script needs to be initialized
            if (!(_scripts[id]->_initialized))
                _scripts[id]->_runInit();

            _scripts[id]->_runBase();
        }

        _execqueue.pop();
    }
}

void ExecEnv::runErase() {
    unsigned id;
    while (!(_erasequeue.empty())) {
        id = _erasequeue.front();
        
        // check if ID is active
        if (_ids.at(id)) {
            // check if script needs to be killed
            if (!(_scripts[id]->_killed))
                _scripts[id]->_runKill();

            _erase(id);
        }

        _erasequeue.pop();
    }
}