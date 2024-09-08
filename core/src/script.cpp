#include "../include/script.hpp"

Script::Script(Script &&other) {
    operator=(std::move(other));
}
Script::Script() :
    _last_execqueue(-1),
    _initialized(false), 
    _killed(false), 
    _exec_enqueued(false), 
    _kill_enqueued(false), 
    _executor(nullptr),
    _executor_id(-1),
    _scriptmanager(nullptr),
    _scriptmanager_id(0),
    _scriptmanager_removeonkill(false)
{}
Script::~Script() {
    // try removing from existing executor
    scriptClear();
}

Script &Script::operator=(Script &&other) {
    if (this != &other) {
        _last_execqueue = other._last_execqueue;
        _initialized = other._initialized;
        _killed = other._killed;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._exec_enqueued;
        _executor = other._executor;
        _executor_id = other._executor_id;
        _scriptmanager = other._scriptmanager;
        _scriptmanager_id = other._scriptmanager_id;
        _scriptmanager_removeonkill = other._scriptmanager_removeonkill;
        other._initialized = false; 
        other._killed = false;
        other._exec_enqueued = false; 
        other._kill_enqueued = false;
        other._executor = nullptr;
        other._executor_id = -1;
        other._scriptmanager = nullptr;
        other._scriptmanager_id = -1;
        other._scriptmanager_removeonkill = false;
    }
    return *this;
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
    
    // this flag could have only been set if an owning ScriptManager sets it, so the reference must be valid, and ID >= 0
    if (_scriptmanager_removeonkill)
            _scriptmanager->remove(_scriptmanager_id);
}

void Script::scriptSetup(Executor *executor) {
    if (!executor)
        throw std::runtime_error("Attempt to setup Script with null Executor reference");

    // try removing from existing executor
    scriptClear();
    
    _executor = executor;
    _executor_id = _executor->push(this);

    scriptResetFlags();
}

void Script::scriptResetFlags() {
    // reset flags
    _initialized = false;
    _killed = false;
    _exec_enqueued = false;
    _kill_enqueued = false;
}

void Script::scriptClear() {
    // reset executor members
    if (_executor && _executor_id >= 0)
        _executor->remove(_executor_id);
    
    _executor = nullptr;
    _executor_id = -1;
    _last_execqueue = -1;
    scriptResetFlags();
}

int Script::getLastExecQueue() { return _last_execqueue; }
bool Script::getInitialized() { return _initialized; }
bool Script::getKilled() { return _killed; }
bool Script::getExecEnqueued() { return _exec_enqueued; }
bool Script::getKillEnqueued() { return _kill_enqueued; }
int Script::getGroup() { return _group; }

void Script::enqueueExec(unsigned queue) {
    if (!_killed) {
        if (_executor)
            _executor->enqueueExec(_executor_id, queue);
        else
            throw std::runtime_error("Attempt to enqueue Script with null Executor reference");
    }
}

void Script::enqueueKill() {
    if (!_killed) {
        if (_executor)
            _executor->enqueueKill(_executor_id);
        else
            throw std::runtime_error("Attempt to kill Script without null Executor reference");
    }
}

ScriptManager *Script::getManager() { return _scriptmanager; }

unsigned Script::getManagerID() { return _scriptmanager_id; }

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
        _scripts = other._scripts;
        _queuepairs = other._queuepairs;
        _push_killqueue = other._push_killqueue;
        _run_killqueue = other._run_killqueue;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;

        // safe as there are no deallocations
        other.uninit();
    }
    return *this;
}

void Executor::init(unsigned max_count, unsigned queues) {
    if (_initialized)
        throw InitializedException();

    _scripts = std::vector<Script*>(max_count, nullptr);
    _queuepairs = std::vector<QueuePair>(queues, QueuePair{});
    _max_count = max_count;
    _count = 0;
    _initialized = true;
}

void Executor::uninit() {
    if (!_initialized)
        return;

    _ids.clear();
    _scripts.clear();
    _queuepairs.clear();
    _max_count = 0;
    _count = 0;
    _initialized = false;
}

int Executor::push(Script *script) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_count >= _max_count)
        throw CountLimitException();
    
    // get a new unique ID
    int id = _ids.push();

    // store script
    _scripts[id] = script;

    _count++;
    return id;
}

void Executor::remove(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    _ids.remove(id);

    _count--;
}

Script* const Executor::get(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        // return "view" of stored Script
        return _scripts[id];
    else
        throw InactiveIDException();
    
    return nullptr;
}

bool Executor::hasID(unsigned id) {
    // check bounds
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");

    return _ids.at(id);
}

void Executor::enqueueExec(unsigned id, unsigned queue) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("ID index out of range");
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");

    if (_ids.at(id)) {
        Script *script = _scripts[id];
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
        Script *script = _scripts[id];
        if (!(script->_kill_enqueued)) {
            // push to kill queue
            _push_killqueue.push(id);
            script->_kill_enqueued = true;
        }
    } else
        throw InactiveIDException();

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
            script = _scripts[id];
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
            script = _scripts[id];

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

bool Executor::getInitialized() { return _initialized; }

int Executor::getQueueCount() { return _queuepairs.size(); }

// --------------------------------------------------------------------------------------------------------------------------

ScriptManager::ScriptManager(unsigned max_count, Executor *executor) :
    _count(0),
    _initialized(false)
{
    init(max_count, executor);
}
ScriptManager::ScriptManager(ScriptManager &&other) {
    operator=(std::move(other));
}
ScriptManager::ScriptManager() :
    _executor(nullptr),
    _max_count(0),
    _count(0),
    _initialized(false)
{}
ScriptManager::~ScriptManager() { /* automatic destruction is fine */ }

ScriptManager &ScriptManager::operator=(ScriptManager &&other) {
    if (this != &other) {
        _scriptinfos = other._scriptinfos;
        _scriptvalues = other._scriptvalues;
        _scriptenqueues = other._scriptenqueues;
        _executor = other._executor;
        _ids = other._ids;
        _scripts = std::move(other._scripts);
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;

        // safe as the only allocated memory (_scripts elements) should be moved already
        other.uninit();
    }
    return *this;
}

void ScriptManager::_scriptSetup(Script *script, ScriptInfo &info, unsigned id, int queue) {
    // set up script
    script->scriptSetup(_executor);
    script->_group = info._group;

    // set up internal fields
    script->_scriptmanager = this;
    script->_scriptmanager_id = id;
    script->_scriptmanager_removeonkill = info._force_removeonkill;

    // enqueue if non-negative queue provided
    if (queue >= 0)
        script->enqueueExec(queue);
    
    _count++;
}

void ScriptManager::_scriptRemoval(ScriptValues &values) {
    values._script_ref->scriptClear();
    _ids.remove(values._manager_id);
    _count--;

    values = ScriptValues{0, nullptr, nullptr, -1};
}

void ScriptManager::init(unsigned max_count, Executor *executor) {
    if (_initialized)
        throw InitializedException();
    
    if (!executor)
        throw std::runtime_error("Attempt to initialize with null Executor reference");

    _max_count = max_count;
    for (unsigned i = 0; i < max_count; i++) {
        _scripts.push_back(std::unique_ptr<Script>(nullptr));
        _scriptvalues.push_back(ScriptValues{0, nullptr, nullptr, -1});
    }

    _executor = executor;
    _initialized = true;
}

void ScriptManager::uninit() {
    if (!_initialized)
        return;
    
    std::queue<ScriptEnqueue> empty;

    _scriptinfos.clear();
    _scriptvalues.clear();
    std::swap(_scriptenqueues, empty);
    _executor = nullptr;
    _ids.clear();
    _scripts.clear();
    _max_count = 0;
    _count = 0;
    _initialized = false;
}

unsigned ScriptManager::spawnScript(const char *scriptname, int executor_queue) {
    // fail if exceeding max size
    if (_count >= _max_count)
        throw CountLimitException();

    // get type information
    ScriptInfo info;
    info = _scriptinfos[scriptname];

    // push to internal storage
    Script *script = info._allocator();
    unsigned id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(script);
    _scriptvalues[id] = ScriptValues{id, scriptname, script, info._group};

    // set up script internals
    _scriptSetup(script, info, id, executor_queue);
    
    // call callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(id);

    return id;
}

void ScriptManager::spawnScriptEnqueue(const char *script_name, int executor_queue) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{true, script_name, executor_queue});
}

std::vector<unsigned> ScriptManager::runSpawnQueue() {
    std::vector<unsigned> ids;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();

        if (scriptenqueue._valid)
            ids.push_back(spawnScript(scriptenqueue._name.c_str(), scriptenqueue._executor_queue));
        
        _scriptenqueues.pop();
    }

    return ids;
}

void ScriptManager::remove(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (!_ids.at(id))
        throw InactiveIDException();

    // get values
    ScriptValues &scriptvalues = _scriptvalues[id];

    // try removal callback
    ScriptInfo &scriptinfo = _scriptinfos[scriptvalues._manager_name];
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(id);

    // remove from script-related systems
    _scriptRemoval(scriptvalues);
}

void ScriptManager::addScript(std::function<Script*(void)> allocator, const char *name, int group, bool force_removeonkill, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)> remove_callback) {  
    if (!hasAddedScript(name))
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

bool ScriptManager::hasAddedScript(const char *scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

bool ScriptManager::hasID(unsigned id) { return _ids.at(id); }

Script *ScriptManager::getScript(unsigned id) {
    // check bounds
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        // return "view" of stored Script
        return _scriptvalues[id]._script_ref;

    throw InactiveIDException();
}

std::vector<unsigned> ScriptManager::getAllByGroup(int group) {
    std::vector<unsigned> ids;
    for (unsigned i = 0; i < _count; i++)
        if (_scriptvalues[i]._manager_id >= 0)
            if (_scriptvalues[i]._group == group)
                ids.push_back(i);
    return ids;
}

std::string ScriptManager::getName(unsigned id) {
    // check bounds
    if (id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id))
        return _scriptvalues[id]._manager_name;

    throw InactiveIDException();
}

unsigned ScriptManager::getCount() { return _count; }

int ScriptManager::getMaxID() { return _ids.size(); }

bool ScriptManager::getInitialized() { return _initialized; }