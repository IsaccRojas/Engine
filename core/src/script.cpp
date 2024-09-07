#include "../include/script.hpp"

Script::Script(Script &&other) {
    operator=(std::move(other));
}
Script::Script() :
    _initialized(false), 
    _killed(false), 
    _exec_enqueued(false), 
    _kill_enqueued(false), 
    _executor(nullptr),
    _executor_id(-1),
    _scriptmanager(nullptr),
    _scriptmanager_id(-1),
    _scriptmanager_removeonkill(false)
{}
Script::~Script() {
    // try removing from existing executor
    scriptClear();
}

Script &Script::operator=(Script &&other) {
    if (this != &other) {
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
    if (_scriptmanager_removeonkill)
        if (_scriptmanager)
            if (_scriptmanager_id >= 0)
                getManager()->remove(_scriptmanager_id);

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
    if (_executor)
        if (_executor_id >= 0)
            _executor->remove(_executor_id);
    
    _executor = nullptr;
    _executor_id = -1;

    scriptResetFlags();
}

void Script::setInitialized(bool initialized) { _initialized = initialized; }
bool Script::getInitialized() { return _initialized; }
void Script::setKilled(bool killed) { _killed = killed; }
bool Script::getKilled() { return _killed; }
void Script::setExecEnqueued(bool exec_enqueued) { _exec_enqueued = exec_enqueued; }
bool Script::getExecEnqueued() { return _exec_enqueued; }
void Script::setKillEnqueued(bool kill_enqueued) { _kill_enqueued = kill_enqueued; }
bool Script::getKillEnqueued() { return _kill_enqueued; }
void Script::setGroup(int group) { _group = group; }
int Script::getGroup() { return _group; }

void Script::enqueueExec() {
    if (!_killed) {
        if (_executor)
            _executor->enqueueExec(_executor_id);
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

// --------------------------------------------------------------------------------------------------------------------------

Executor::Executor(unsigned max_count) : _initialized(false) {
    init(max_count);
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
        _push_execqueue = other._push_execqueue;
        _run_execqueue = other._run_execqueue;
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

void Executor::init(unsigned max_count) {
    if (_initialized)
        throw InitializedException();

    _scripts = std::vector<Script*>(max_count, nullptr);
    _max_count = max_count;
    _count = 0;
    _initialized = true;
}

void Executor::uninit() {
    if (!_initialized)
        return;
    
    // queues to be deallocated when they go out of context
    std::queue<unsigned> empty1;
    std::queue<unsigned> empty2;
    std::queue<unsigned> empty3;
    std::queue<unsigned> empty4;

    _ids.clear();
    _scripts.clear();
    std::swap(_push_execqueue, empty1);
    std::swap(_run_execqueue, empty2);
    std::swap(_push_killqueue, empty3);
    std::swap(_run_killqueue, empty4);
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

void Executor::enqueueExec(unsigned id) {
    // check bounds
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getExecEnqueued())) {
            _push_execqueue.push(id);
            script->setExecEnqueued(true);
        }
    } else
        throw InactiveIDException();
}

void Executor::enqueueKill(unsigned id) {
    // check bounds
    if (id < 0 || id >= _ids.size())
        throw std::out_of_range("Index out of range");

    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getKillEnqueued())) {
            _push_killqueue.push(id);
            script->setKillEnqueued(true);
        }
    } else
        throw InactiveIDException();

}

void Executor::runExecQueue() {
    // swap queues
    _run_execqueue.swap(_push_execqueue);

    unsigned id;
    Script *script;
    while (!(_run_execqueue.empty())) {
        id = _run_execqueue.front();

        // check bounds
        if (id < 0 || id >= _ids.size())
            throw std::out_of_range("Index out of range");

        // check if ID is active
        if (_ids.at(id)) {

            script = _scripts[id];
            script->setExecEnqueued(false);

            // check if script needs to be initialized
            if (!(script->getInitialized())) {
                script->runInit();
                script->setInitialized(true);
            }

            // check if script hasn't been killed yet
            if (!(script->getKilled()))
                script->runBase();

        } else
            throw InactiveIDException();
        
        _run_execqueue.pop();
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
            script->setKillEnqueued(false);

            // check if script needs to be killed
            if (!(script->getKilled())) {
                script->runKill();
                script->setKilled(true);
            }

        } else
            throw InactiveIDException();
        
        _run_killqueue.pop();
    }
}

bool Executor::getInitialized() { return _initialized; }

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

void ScriptManager::_scriptSetup(Script *script, ScriptInfo &info, unsigned id) {
    // set up script
    script->scriptSetup(_executor);
    script->setGroup(info._group);

    // set up internal fields
    script->_scriptmanager = this;
    script->_scriptmanager_id = id;
    script->_scriptmanager_removeonkill = info._force_removeonkill;

    // enqueue if set
    if (info._force_executorenqueue)
        script->enqueueExec();
    
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

unsigned ScriptManager::spawnScript(const char *scriptname) {
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
    _scriptSetup(script, info, id);
    
    // call callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(script);

    return id;
}

void ScriptManager::spawnScriptEnqueue(const char *script_name) {
    // names are not checked here for the sake of efficiency
    _scriptenqueues.push(ScriptEnqueue{true, script_name});
}

std::vector<unsigned> ScriptManager::runSpawnQueue() {
    std::vector<unsigned> ids;
    
    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue &scriptenqueue = _scriptenqueues.front();

        if (scriptenqueue._valid)
            ids.push_back(spawnScript(scriptenqueue._name.c_str()));
        
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

    // get info
    ScriptValues &values = _scriptvalues[id];

    // remove from script-related systems
    _scriptRemoval(values);
}

void ScriptManager::addScript(std::function<Script*(void)> allocator, const char *name, int group, bool force_executorenqueue, bool force_removeonkill, std::function<void(Script*)> spawn_callback) {  
    if (!hasAddedScript(name))
        _scriptinfos[name] = ScriptInfo{
            group,
            force_executorenqueue,
            force_removeonkill,
            allocator,
            spawn_callback
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