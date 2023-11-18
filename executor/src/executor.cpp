#include "executor.hpp"

Script::Script() :
    _initialized(false), 
    _killed(false), 
    _execqueued(false), 
    _killqueued(false), 
    _executor_ready(false), 
    _executor_id(-1), 
    _hasmanager(false),
    _manager(nullptr)
{}
Script::~Script() {
    // try removing from existing executor
    if (_executor_ready)
        if (_executor_id >= 0)
            _executor->erase(_executor_id);
}

void Script::_init() {}
void Script::_base() {}
void Script::_kill() {}

void Script::_runInit() {
    _init();

}
void Script::_runBase() {
    _base();
}

void Script::_runKill() {
    _kill();
}

void Script::scriptSetup(Executor *executor) {
    // try removing from existing executor
    if (_executor_ready)
        if (_executor_id >= 0)
            _executor->erase(_executor_id);
    
    _executor = executor;
    _executor_id = _executor->push(this);

    _executor_ready = true;
}

void Script::enqueue() {
    if (_executor_ready)
        _executor->queueExec(_executor_id);
}

void Script::kill() {
    if (_executor_ready)
        _executor->queueKill(_executor_id);
}

ManagerPtr *Script::getManager() { return _manager; }
bool Script::hasManager() { return _hasmanager; }

// --------------------------------------------------------------------------------------------------------------------------

Executor::Executor(int maxcount) : 
    _maxcount(maxcount),
    _scripts(maxcount, nullptr)
{}

Executor::~Executor() {}

int Executor::push(Script *script) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillsize() >= _maxcount)
        return -1;
    
    // get a new unique ID
    int id = _ids.push();
    
    // initialize flags
    script->_initialized = false;
    script->_killed = false;
    script->_execqueued = false;
    script->_killqueued = false;

    // store script
    _scripts[id] = script;

    return id;
}

void Executor::erase(int id) {
    // free ID from partitioner
    if (_ids.at(id)) {
        Script *script = _scripts[id];
        script->_initialized = false;
        script->_killed = false;
        script->_execqueued = false;
        script->_killqueued = false;
        _ids.erase_at(id);
    }
}

Script* const Executor::get(int id) {
    if (_ids.at(id))
        // return "view" of stored Script
        return _scripts[id];
    else
        return nullptr;
}

bool Executor::has(int id) {
    return _ids.at(id);
}

void Executor::queueExec(int id) {
    // if id exists, and not already queued, queue
    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->_execqueued)) {
            _pushexecqueue.push(id);
            script->_execqueued = true;
        }
    }
}

void Executor::queueKill(int id) {
    // if id exists, and not already killed, queue
    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->_killqueued)) {
            _pushkillqueue.push(id);
            script->_killqueued = true;
        }
    }
}

void Executor::runExec() {
    // swap queues
    _runexecqueue.swap(_pushexecqueue);

    unsigned id;
    Script *script;
    while (!(_runexecqueue.empty())) {
        id = _runexecqueue.front();

        // check if ID is active
        if (_ids.at(id)) {
            script = _scripts[id];

            // check if script needs to be initialized
            if (!(script->_initialized)) {
                script->_runInit();
                script->_initialized = true;
            }

            script->_execqueued = false;
            script->_runBase();
        }

        _runexecqueue.pop();
    }
}

void Executor::runKill() {
    // swap queues
    _runkillqueue.swap(_pushkillqueue);

    unsigned id;
    Script *script;
    while (!(_runkillqueue.empty())) {
        id = _runkillqueue.front();
        
        // check if ID is active
        if (_ids.at(id)) {
            script = _scripts[id];

            // check if script needs to be killed
            if (!(script->_killed)) {
                script->_killqueued = false;
                script->_runKill();
                script->_killed = true;
            }
        }

        _runkillqueue.pop();
    }
}