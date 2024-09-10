#include "../include/script.hpp"

Script::Script(Script &&other) {
    operator=(std::move(other));
}
Script::Script() :
    _executor(nullptr),
    _executor_id(-1),
    _last_execqueue(-1),
    _remove_onkill(false),
    _initialized(false), 
    _killed(false), 
    _exec_enqueued(false), 
    _kill_enqueued(false),
    _group(-1)
{}
Script::~Script() {
    // try removing from existing executor
    _scriptRemove();
}

Script &Script::operator=(Script &&other) {
    if (this != &other) {
        _executor = other._executor;
        _executor_id = other._executor_id;
        _last_execqueue = other._last_execqueue;
        _remove_onkill = other._remove_onkill;
        _initialized = other._initialized;
        _killed = other._killed;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._exec_enqueued;
        _group = other._group;
        other._executor = nullptr;
        other._executor_id = -1;
        other._last_execqueue = -1;
        other._remove_onkill = false;
        other._initialized = false;
        other._killed = false;
        other._exec_enqueued = false;
        other._exec_enqueued = false;
        other._group = -1;
    }
    return *this;
}

void Script::_scriptRemove() {
    // remove from owned executor
    if (_executor && _executor_id >= 0)
        _executor->remove(_executor_id);
}

void Script::runInit() {
    _init();

}
void Script::runBase() {
    _base();
}

void Script::runKill() {
    _kill();
    
    // this flag could have only been set if an owning ScriptManager sets it, so the reference must be valid, and ID >= 0
    if (_remove_onkill)
        _scriptRemove();
}

int Script::getLastExecQueue() { return _last_execqueue; }
bool Script::getInitialized() { return _initialized; }
bool Script::getKilled() { return _killed; }
bool Script::getExecEnqueued() { return _exec_enqueued; }
bool Script::getKillEnqueued() { return _kill_enqueued; }
int Script::getGroup() { return _group; }

void Script::enqueueExec(unsigned queue) {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for execution with null Executor owner");
    if (!_killed)
        _executor->enqueueExec(_executor_id, queue);
}

void Script::enqueueKill() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for kill with null Executor owner");
    if (!_killed)
        _executor->enqueueKill(_executor_id);
}

// --------------------------------------------------------------------------------------------------------------------------

Executor::Executor(unsigned max_count, unsigned queues) : _initialized(false) {
    init(max_count, queues);
}
Executor::Executor(Executor &&other) {
    operator=(std::move(other));
}
Executor::Executor() : _max_count(0), _count(0), _initialized(false) {}

Executor::~Executor() { /* automatic destruction is fine */ }

Executor &Executor::operator=(Executor &&other) {
    if (this != &other) {
        _ids = other._ids;
        _scripts = std::move(other._scripts);
        _queuepairs = other._queuepairs;
        _push_killqueue = other._push_killqueue;
        _run_killqueue = other._run_killqueue;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;

        // safe as there are no deallocations (unique_ptrs should be moved by this point)
        other.uninit();
    }
    return *this;
}

unsigned Executor::_spawnScript(const char *scriptname, int execution_queue, int tag) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();

    // get type information
    ScriptInfo &info = _scriptinfos[scriptname];

    // push to storage
    Script *script = info._allocator->_allocate(tag);
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(script);
    _scriptvalues[id] = ScriptValues{scriptname, info._group};

    // set up script
    script->_executor = this;
    script->_executor_id = id;
    script->_remove_onkill = info._force_removeonkill;
    script->_group = info._group;

    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(id, execution_queue);
    
    _count++;
    
    // try spawn callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(id);

    return id;
}

void Executor::init(unsigned max_count, unsigned queues) {
    if (_initialized)
        throw InitializedException();

    _scripts = std::vector<std::unique_ptr<Script>>(max_count, std::unique_ptr<Script>(nullptr));
    _scriptvalues = std::vector<ScriptValues>(max_count, {nullptr, -1});
    _queuepairs = std::vector<QueuePair>(queues, QueuePair{});
    _max_count = max_count;
    _count = 0;
    _initialized = true;
}

void Executor::uninit() {
    if (!_initialized)
        return;

    std::queue<ScriptEnqueue> empty;
    
    _scripts.clear();
    _scriptvalues.clear();
    _queuepairs.clear();
    _max_count = 0;
    _count = 0;
    _ids.clear();
    _scriptinfos.clear();
    std::swap(_scriptenqueues, empty);
    _initialized = false;
}

Script *Executor::get(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        return _scripts[id].get();

    throw InactiveIDException();
}

void Executor::remove(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (!_ids.at(id))
        throw InactiveIDException();

    // get values and info
    ScriptValues &scriptvalues = _scriptvalues[id];
    ScriptInfo &scriptinfo = _scriptinfos[scriptvalues._manager_name];

    // try removal callback if it exists
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(id);

    // unset Script's fields and ScriptValues
    _scripts[id]->_executor = nullptr;
    _scripts[id]->_executor_id = -1;
    scriptvalues = ScriptValues{nullptr, -1};

    _ids.remove(id);
    _count--;
}

void Executor::add(AllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)> remove_callback) {  
    if (!hasAdded(name))
        _scriptinfos[name] = ScriptInfo{
            group,
            force_removeonkill,
            allocator,
            spawn_callback,
            remove_callback
        };

    else
        throw std::runtime_error("Attempt to add already added Script name");
}

void Executor::enqueueSpawn(const char *script_name, int execution_queue, int tag) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{script_name, execution_queue, tag});
}

void Executor::enqueueExec(unsigned id, unsigned queue) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("ID index out of range");
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");

    if (_ids.at(id)) {
        Script *script = _scripts[id].get();
        if (!(script->_exec_enqueued)) {
            // push to specified pair
            _queuepairs[queue]._push_execqueue.push(id);
            script->_exec_enqueued = true;
        }
    } else
        throw InactiveIDException();
}

void Executor::enqueueKill(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id)) {
        Script *script = _scripts[id].get();
        if (!(script->_kill_enqueued)) {
            // push to kill queue
            _push_killqueue.push(id);
            script->_kill_enqueued = true;
        }
    } else
        throw InactiveIDException();

}

std::vector<unsigned> Executor::runSpawnQueue() {
    std::vector<unsigned> ids;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();
        ids.push_back(_spawnScript(scriptenqueue._name.c_str(), scriptenqueue._execution_queue, scriptenqueue._tag));
        _scriptenqueues.pop();
    }

    return ids;
}

void Executor::runExecQueue(unsigned queue) {
    // check bounds
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");
    
    std::queue<unsigned> &push_execqueue = _queuepairs[queue]._push_execqueue;
    std::queue<unsigned> &run_execqueue = _queuepairs[queue]._run_execqueue;
    
    // swap queues
    run_execqueue.swap(push_execqueue);

    unsigned id;
    Script *script;
    while (!(run_execqueue.empty())) {
        id = run_execqueue.front();

        // check bounds
        if (id < 0 || id >= _ids.size())
            throw std::out_of_range("Index out of range");

        // check if ID is active
        if (_ids.at(id)) {
            script = _scripts[id].get();
            script->_last_execqueue = queue;
            script->_exec_enqueued = false;

            // check if script hasn't been killed yet
            if (!(script->_killed)) {
                // check if script needs to be initialized
                if (!(script->_initialized)) {
                    script->runInit();
                    script->_initialized = true;
                }

                script->runBase();
            }

        } else
            throw InactiveIDException();
        
        run_execqueue.pop();
    }
}

void Executor::runKillQueue() {
    // swap queues
    _run_killqueue.swap(_push_killqueue);

    unsigned id;
    Script *script;

    while (!(_run_killqueue.empty())) {
        id = _run_killqueue.front();

        // check bounds
        if (id < 0 || id >= _ids.size())
            throw std::out_of_range("Index out of range");
        
        // check if ID is active
        if (_ids.at(id)) {
            script = _scripts[id].get();

            // check if script needs to be killed
            if (!(script->_killed)) {
                script->runKill();
                script->_killed = true;
            }
            
            script->_kill_enqueued = false;
        } else
            throw InactiveIDException();
        
        _run_killqueue.pop();
    }
}

bool Executor::hasAdded(const char *scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

bool Executor::hasID(unsigned id) { return _ids.at(id); }

std::vector<unsigned> Executor::getAllByGroup(int group) {
    std::vector<unsigned> ids;
    for (unsigned i = 0; i < _ids.size(); i++)
        if (_ids.at(i))
            if (_scriptvalues[i]._group == group)
                ids.push_back(i); 
    return ids;
}

std::string Executor::getName(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        return _scriptvalues[id]._manager_name;

    throw InactiveIDException();
}

bool Executor::getInitialized() { return _initialized; }

unsigned Executor::getCount() { return _count; }

int Executor::getMaxID() { return _ids.size(); }

int Executor::getQueueCount() { return _queuepairs.size(); }