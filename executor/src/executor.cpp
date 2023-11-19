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
    scriptResetExec();
}

void Script::_init() {}
void Script::_base() {}
void Script::_kill() {}

void Script::runInit() {
    _init();

}
void Script::runBase() {
    _base();
}

void Script::runKill() {
    _kill();
}

void Script::scriptSetup(Executor *executor) {
    // try removing from existing executor
    scriptResetExec();
    
    _executor = executor;
    _executor_id = _executor->push(this);

    _executor_ready = true;

    scriptResetFlags();
}

void Script::scriptResetFlags() {
    // reset flags
    _initialized = false;
    _killed = false;
    _execqueued = false;
    _killqueued = false;
}

void Script::scriptResetExec() {
    // reset executor fields
    if (_executor_ready)
        if (_executor_id >= 0)
            _executor->erase(_executor_id);
    
    _executor = nullptr;
    _executor_id = -1;
    _executor_ready = false;
}

void Script::setInitialized(bool initialized) { _initialized = initialized; }
bool Script::getInitialized() { return _initialized; }
void Script::setKilled(bool killed) { _killed = killed; }
bool Script::getKilled() { return _killed; }
void Script::setExecQueued(bool execqueued) { _execqueued = execqueued; }
bool Script::getExecQueued() { return _execqueued; }
void Script::setKillQueued(bool killqueued) { _killqueued = killqueued; }
bool Script::getKillQueued() { return _killqueued; }
void Script::setType(int type) { _type = type; }
int Script::getType() { return _type; }

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
    if (_ids.fillsize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Executor " << this << std::endl;
        return -1;
    }
    
    // get a new unique ID
    int id = _ids.push();

    // store script
    _scripts[id] = script;

    return id;
}

void Executor::erase(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Executor " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Executor " << this << std::endl;
        return;
    }

    if (id >= 0 && _ids.at(id))
        _ids.erase_at(id);
}

Script* const Executor::get(int id) {
    if (id >= 0 && _ids.at(id))
        // return "view" of stored Script
        return _scripts[id];
    else
        return nullptr;
}

bool Executor::has(int id) {
    if (id >= 0)
        return _ids.at(id);
    return false;
}

void Executor::queueExec(int id) {
    // if id exists, and not already queued, queue
    if (id >= 0 && _ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getExecQueued())) {
            _pushexecqueue.push(id);
            script->setExecQueued(true);
        }
    }
}

void Executor::queueKill(int id) {
    // if id exists, and not already killed, queue
    if (id >= 0 && _ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getKillQueued())) {
            _pushkillqueue.push(id);
            script->setKillQueued(true);
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
        if (id >= 0 && _ids.at(id)) {
            script = _scripts[id];

            // check if script needs to be initialized
            if (!(script->getInitialized())) {
                script->runInit();
                script->setInitialized(true);
            }

            script->setExecQueued(false);
            script->runBase();
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
        if (id >= 0 && _ids.at(id)) {
            script = _scripts[id];

            // check if script needs to be killed
            if (!(script->getKilled())) {
                script->setKillQueued(false);
                script->runKill();
                script->setKilled(true);
            }
        }

        _runkillqueue.pop();
    }
}