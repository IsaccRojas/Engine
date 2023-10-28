#include "execenv.hpp"

ExecEnv::ExecEnv(int maxcount) : 
    _maxcount(maxcount)
{
    // initialize _maxcount unique_ptrs to Scripts
    for (int i = 0; i < _maxcount; i++)
        _scripts.push_back(std::unique_ptr<Script>(nullptr));
}

ExecEnv::~ExecEnv() {}

void ExecEnv::add(std::function<Script*(void)> allocator, const char *name) {
    _scripttypes[name] = _ScriptType{allocator, name};
}

int ExecEnv::push(Script *script) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillsize() >= _maxcount)
        return -1;
    
    // get a new unique ID
    int id = _ids.push();

    // allocate new script
    _scripts[id] = std::unique_ptr<Script>(script);
    
    // initialize owner, ID and flags
    _scripts[id]->_owner = this;
    _scripts[id]->_id = id;
    _scripts[id]->_initialized = false;
    _scripts[id]->_killed = false;

    return id;
}
int ExecEnv::push(const char *name) {
    // call push() with pointer returned from stored allocator
    return push(_scripttypes[name].allocator());
}

void ExecEnv::_erase(int id) {
    // free ID from partitioner
    _ids.erase_at(id);
}

Script* const ExecEnv::get(int id) {
    if (_ids.at(id))
        // return "view" of stored Script
        return _scripts[id].get();
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
                _scripts[id]->init();

            _scripts[id]->base();
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
                _scripts[id]->kill();

            _erase(id);
        }

        _erasequeue.pop();
    }
}