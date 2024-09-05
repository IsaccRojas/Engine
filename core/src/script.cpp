#include "../include/script.hpp"

Script::Script(Script &&other) {
    operator=(std::move(other));
}
Script::Script() :
    _initialized(false), 
    _killed(false), 
    _exec_queued(false), 
    _kill_queued(false), 
    _executor(nullptr),
    _executor_id(-1),
    _executor_ready(false),
    _scriptmanager(nullptr),
    _scriptmanager_id(-1),
    _scriptmanager_ready(false),
    _scriptmanager_removeonkill(false)
{}
Script::~Script() {
    // try removing from existing executor
    scriptResetExec();
}

Script &Script::operator=(Script &&other) {
    if (&other != this) {
        _initialized = other._initialized;
        _killed = other._killed;
        _exec_queued = other._exec_queued;
        _kill_queued = other._exec_queued;
        _executor = other._executor;
        _executor_id = other._executor_id;
        _executor_ready = other._executor_ready;
        _scriptmanager = other._scriptmanager;
        _scriptmanager_id = other._scriptmanager_id;
        _scriptmanager_ready = other._scriptmanager_ready;
        _scriptmanager_removeonkill = other._scriptmanager_removeonkill;
        other._initialized = false; 
        other._killed = false;
        other._exec_queued = false; 
        other._kill_queued = false;
        other._executor = nullptr;
        other._executor_id = -1;
        other._executor_ready = false;
        other._scriptmanager = nullptr;
        other._scriptmanager_id = -1;
        other._scriptmanager_ready = false;
        other._scriptmanager_removeonkill = false;
    }
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
        if (_scriptmanager_ready)
            if (_scriptmanager)
                if (_scriptmanager_id >= 0)
                    getManager()->remove(_scriptmanager_id);

}

void Script::scriptSetup(Executor *executor) {
    if (!executor) {
        std::cerr << "WARN: Script::scriptSetup: attempt to setup with null executor reference in Script instance " << this << std::endl;
        return;
    }

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
    _exec_queued = false;
    _kill_queued = false;
}

void Script::scriptResetExec() {
    // reset executor fields
    if (_executor_ready)
        if (_executor_id >= 0)
            _executor->remove(_executor_id);
    
    _executor = nullptr;
    _executor_id = -1;
    _executor_ready = false;
}

void Script::setInitialized(bool initialized) { _initialized = initialized; }
bool Script::getInitialized() { return _initialized; }
void Script::setKilled(bool killed) { _killed = killed; }
bool Script::getKilled() { return _killed; }
void Script::setExecQueued(bool exec_queued) { _exec_queued = exec_queued; }
bool Script::getExecQueued() { return _exec_queued; }
void Script::setKillQueued(bool kill_queued) { _kill_queued = kill_queued; }
bool Script::getKillQueued() { return _kill_queued; }
void Script::setGroup(int group) { _group = group; }
int Script::getGroup() { return _group; }

void Script::enqueue() {
    if (_executor_ready)
        _executor->queueExec(_executor_id);
    else
        std::cerr << "WARN: Script::enqueue: attempt to queue self without executor ready in Script instance " << this << std::endl;
}

void Script::kill() {
    if (!_killed) {
        if (_executor_ready)
            _executor->queueKill(_executor_id);
        else
            std::cerr << "WARN: Script::kill: attempt to kill self without executor ready in Script instance " << this << std::endl;
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
}

void Executor::init(unsigned max_count) {
    if (_initialized) {
        std::cerr << "WARN: Executor::init: attempt to initialize already initialized Executor instance " << this << std::endl;
        return;
    }

    _scripts = std::vector<Script*>(max_count, nullptr);
    _max_count = max_count;
    _count = 0;
    _initialized = true;
}

void Executor::uninit() {
    if (!_initialized)
        return;
    
    // queues to be deallocated when they go out of context
    std::queue<int> empty1;
    std::queue<int> empty2;
    std::queue<int> empty3;
    std::queue<int> empty4;

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
    if (!_initialized) {
        std::cerr << "WARN: Executor::push: attempt to push to uninitialized Executor instance " << this << std::endl;
        return -1;
    }

    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_count >= _max_count) {
        std::cerr << "WARN: Executor::push: max count reached in Executor instance " << this << std::endl;
        return -1;
    }
    
    // get a new unique ID
    int id = _ids.push();

    // store script
    _scripts[id] = script;

    _count++;
    return id;
}

void Executor::remove(int id) {
    if (!_initialized) {
        std::cerr << "WARN: Executor::remove: attempt to remove from uninitialized Executor instance " << this << std::endl;
        return;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: Executor::remove: attempt to remove out-of-range ID " << id << " (max ID = " << _ids.size() << ") from Executor instance " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        _ids.remove(id);
        _count--;
    } else
        std::cerr << "WARN: Executor::remove: attempt to remove inactive ID " << id << " in Executor instance " << this << std::endl;
}

Script* const Executor::get(int id) {
    if (!_initialized) {
        std::cerr << "WARN: Executor::get: attempt to get from uninitialized Executor instance " << this << std::endl;
        return nullptr;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: Executor::get: attempt to get reference with out-of-range ID " << id << " (max ID = " << _ids.size() << ") from Executor instance " << this << std::endl;
        return nullptr;
    }

    if (_ids.at(id))
        // return "view" of stored Script
        return _scripts[id];
    else
        std::cerr << "WARN: Executor::get: attempt to reference with inactive ID " << id << " in Executor instance " << this << std::endl;
    
    return nullptr;
}

bool Executor::has(int id) {
    if (!_initialized) {
        std::cerr << "WARN: Executor::has: attempt to call has() on uninitialized Executor instance " << this << std::endl;
        return false;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: Executor::has: attempt to call has() with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in Executor instance " << this << std::endl;
        return false;
    }

    return _ids.at(id);
}

void Executor::queueExec(int id) {
    if (!_initialized) {
        std::cerr << "WARN: Executor::queueExec: attempt to queue for execution in uninitialized Executor instance " << this << std::endl;
        return;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: Executor::queueExec: attempt to queue for execution with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in Executor instance " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getExecQueued())) {
            _push_execqueue.push(id);
            script->setExecQueued(true);
        }
    } else
        std::cerr << "WARN: Executor::queueExec: attempt to queue inactive ID " << id << " for execution in Executor instance " << this << std::endl;
}

void Executor::queueKill(int id) {
    if (!_initialized) {
        std::cerr << "WARN: Executor::queueKill: attempt to queue for killing in uninitialized Executor instance " << this << std::endl;
        return;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: Executor::queueKill: attempt to queue for kill with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in Executor instance " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        Script *script = _scripts[id];
        if (!(script->getKillQueued())) {
            _push_killqueue.push(id);
            script->setKillQueued(true);
        }
    } else
        std::cerr << "WARN: Executor::queueKill: attempt to queue inactive ID " << id << " for kill in Executor instance " << this << std::endl;

}

void Executor::runExec() {
    if (!_initialized) {
        std::cerr << "WARN: Executor::runExec: attempt to run execution queue in uninitialized Executor instance " << this << std::endl;
        return;
    }

    // swap queues
    _run_execqueue.swap(_push_execqueue);

    unsigned id;
    Script *script;
    while (!(_run_execqueue.empty())) {
        id = _run_execqueue.front();

        // check bounds
        if (id < 0 || id >= _ids.size()) {
            std::cerr << "WARN: Executor::runExec: attempt to run execution with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in Executor instance " << this << std::endl;
            return;
        }

        // check if ID is active
        if (_ids.at(id)) {

            script = _scripts[id];
            script->setExecQueued(false);

            // check if script needs to be initialized
            if (!(script->getInitialized())) {
                script->runInit();
                script->setInitialized(true);
            }

            // check if script hasn't been killed yet
            if (!(script->getKilled()))
                script->runBase();

        } else
            std::cerr << "WARN: Executor::runExec: attempt to execute inactive ID " << id << " in Executor instance " << this << std::endl;
        
        _run_execqueue.pop();
    }
}

void Executor::runKill() {
    if (!_initialized) {
        std::cerr << "WARN: Executor::runKill: attempt to run kill queue in uninitialized Executor instance " << this << std::endl;
        return;
    }

    // swap queues
    _run_killqueue.swap(_push_killqueue);

    unsigned id;
    Script *script;
    while (!(_run_killqueue.empty())) {
        id = _run_killqueue.front();

        // check bounds
        if (id < 0 || id >= _ids.size()) {
            std::cerr << "WARN: Executor::runKill: attempt to queue for kill with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in Executor instance " << this << std::endl;
            return;
        }
        
        // check if ID is active
        if (_ids.at(id)) {

            script = _scripts[id];
            script->setKillQueued(false);

            // check if script needs to be killed
            if (!(script->getKilled())) {
                script->runKill();
                script->setKilled(true);
            }

        } else
            std::cerr << "WARN: Executor::runKill: attempt to kill inactive ID " << id << " in Executor instance " << this << std::endl;
        
        _run_killqueue.pop();
    }
}

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
ScriptManager::~ScriptManager() {
    uninit();
}

ScriptManager &ScriptManager::operator=(ScriptManager &&other) {
    if (this != &other) {
        _scriptinfos = other._scriptinfos;
        _scriptvalues = other._scriptvalues;
        _executor = other._executor;
        _ids = other._ids;
        _scripts = other._scripts;
        _max_count = other._max_count;
        _count = other._count;
        _initialized = other._initialized;
        // safe as the only allocated memory (_scripts elements) should be moved already
        other.uninit();
    }
}

void ScriptManager::_scriptSetup(Script *script, ScriptInfo &info, int id) {
    if (!_initialized) {
        std::cerr << "WARN: attempt to setup script in uninitialized ScriptManager instance " << this << std::endl;
        return;
    }

    // set up script
    if (_executor)
        script->scriptSetup(_executor);
    script->setGroup(info._group);

    // set up internal fields
    script->_scriptmanager = this;
    script->_scriptmanager_id = id;
    script->_scriptmanager_removeonkill = info._force_removeonkill;
    script->_scriptmanager_ready = true;

    // enqueue if set
    if (info._force_enqueue)
        script->enqueue();
    
    _count++;
}

void ScriptManager::_scriptRemoval(ScriptValues &values) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::_scriptRemoval: attempt to remove script in uninitialized ScriptManager instance " << this << std::endl;
        return;
    }

    if (!values._script_ref) {
        std::cerr << "WARN: ScriptManager::_scriptRemoval: attempt to remove null script in ScriptManager instance " << this << std::endl;
        return;
    }

    values._script_ref->scriptResetExec();
    _ids.remove(values._manager_id);
    _count--;

    values = ScriptValues{-1, nullptr, nullptr, -1};
}

void ScriptManager::init(unsigned max_count, Executor *executor) {
    if (_initialized) {
        std::cerr << "WARN: ScriptManager::init: attempt to initialize already initialized ScriptManager instance " << this << std::endl;
        return;
    }
    
    if (!_executor) {
        std::cerr << "WARN: ScriptManager::init: attempt to initialize with null executor reference in ScriptManager instance " << this << std::endl;
        return;
    }

    _max_count = max_count;
    for (unsigned i = 0; i < max_count; i++) {
        _scripts.push_back(std::unique_ptr<Script>(nullptr));
        _scriptvalues.push_back(ScriptValues{-1, NULL, nullptr, -1});
    }

    _executor = executor;
    _initialized = true;
}

void ScriptManager::uninit() {
    if (!_initialized)
        return;
    
    _scriptinfos.clear();
    _scriptvalues.clear();
    _executor = nullptr;
    _ids.clear();
    _scripts.clear();
    _max_count = 0;
    _count = 0;
    _initialized = false;
}

bool ScriptManager::hasScript(const char *scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

int ScriptManager::spawnScript(const char *scriptname) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::spawnScript: attempt to spawn Script in uninitialized ScriptManager instance " << this << std::endl;
        return -1;
    }

    // fail if exceeding max size
    if (_count >= _max_count) {
        std::cerr << "WARN: ScriptManager::spawnScript: max count of " << _max_count << " reached in ScriptManager instance " << this << std::endl;
        return -1;
    }

    // get type information
    ScriptInfo info;
    try {
        info = _scriptinfos[scriptname];
    } catch (const std::out_of_range& e) {
        std::cerr << "WARN: ScriptManager::spawnScript: ScriptInfo name '" << scriptname << "' does not exist in ScriptManager instance " << this << std::endl;
        return -1;
    }

    // push to internal storage
    Script *script = info._allocator();
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(script);
    _scriptvalues[id] = ScriptValues{id, scriptname, script, info._group};

    // set up script internals
    _scriptSetup(script, info, id);
    
    // call callback if it exists
    if (info._spawncallback)
        info._spawncallback(script);

    _count++;
    return id;
}

Script *ScriptManager::getScript(int id) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::getScript: attempt to get Script from uninitialized ScriptManager instance " << this << std::endl;
        return nullptr;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: ScriptManager::getScript: attempt to get Script with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in ScriptManager instance " << this << std::endl;
        return nullptr;
    }

    if (_ids.at(id))
        // return "view" of stored Script
        return _scriptvalues[id]._script_ref;
    else
        std::cerr << "WARN: ScriptManager::getScript: attempt to get Script with inactive ID " << id << " in ScriptManager instance " << this << std::endl;

    return nullptr;
}

std::vector<int> ScriptManager::getAllByGroup(int group) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::getAllByGroup: attempt to get scripts by group from uninitialized ScriptManager instance " << this << std::endl;
        return std::vector<int>();
    }

    std::vector<int> ids;
    for (unsigned i = 0; i < _count; i++)
        if (_scriptvalues[i]._manager_id >= 0)
            if (_scriptvalues[i]._group == group)
                ids.push_back(i);
    return ids;
}

std::string ScriptManager::getName(int id) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::getName: attempt to get name of ID from uninitialized ScriptManager instance " << this << std::endl;
        return "";
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: ScriptManager::getName: attempt to get Script name with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in ScriptManager instance " << this << std::endl;
        return "";
    }

    if (_ids.at(id))
        return _scriptvalues[id]._manager_name;
    else
        std::cerr << "WARN: ScriptManager::getName: attempt to get name of inactive ID " << id << " in ScriptManager instance " << this << std::endl;
        
    return "";
}

unsigned ScriptManager::getCount() {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::getCount: attempt to get count of uninitialized ScriptManager instance " << this << std::endl;
        return 0;
    }

    return _count;
}

void ScriptManager::remove(int id) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::remove: attempt to remove from uninitialized ScriptManager instance " << this << std::endl;
        return;
    }

    // check bounds
    if (id < 0 || id >= _ids.size()) {
        std::cerr << "WARN: ScriptManager::remove: attempt to remove Script with out-of-range ID " << id << " (max ID = " << _ids.size() << ") in ScriptManager instance " << this << std::endl;
        return;
    }

    if (!_ids.at(id)) {
        std::cerr << "WARN: ScriptManager::remove: attempt to remove inactive ID " << id << " from ScriptManager instance " << this << std::endl;
        return;
    }

    // get info
    ScriptValues &values = _scriptvalues[id];

    // remove from script-related systems
    _scriptRemoval(values);    
    _scriptvalues[id] = ScriptValues{-1, NULL, nullptr, -1};
    _count--;
}

void ScriptManager::addScript(std::function<Script*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, std::function<void(Script*)> spawn_callback) {
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::addScript: attempt to add script info to uninitialized ScriptManager instance " << this << std::endl;
        return;
    }
    
    if (!hasScript(name))
        _scriptinfos[name] = ScriptInfo{
            group,
            force_enqueue,
            force_removeonkill,
            allocator,
            spawn_callback
        };
}

int ScriptManager::getMaxID() { 
    if (!_initialized) {
        std::cerr << "WARN: ScriptManager::getMaxID: attempt to get max ID from uninitialized ScriptManager instance " << this << std::endl;
        return;
    }

    return _ids.size();
}