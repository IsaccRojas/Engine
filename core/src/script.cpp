#include "../include/script.hpp"

Script::Script() :
    _initialized(false), 
    _killed(false), 
    _execqueued(false), 
    _killqueued(false), 
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
            _executor->remove(_executor_id);
    
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

ScriptManager *Script::getManager() { return _scriptmanager; }

// --------------------------------------------------------------------------------------------------------------------------

Executor::Executor(unsigned maxcount) :
    _scripts(maxcount, nullptr),
    _maxcount(maxcount)
{}

Executor::~Executor() {}

int Executor::push(Script *script) {
    // if number of active IDs is greater than or equal to maximum allowed count, return -1
    if (_ids.fillSize() >= _maxcount) {
        std::cerr << "WARN: limit reached in Executor " << this << std::endl;
        return -1;
    }
    
    // get a new unique ID
    int id = _ids.push();

    // store script
    _scripts[id] = script;

    return id;
}

void Executor::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Executor " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Executor " << this << std::endl;
        return;
    }

    if (id >= 0 && _ids.at(id))
        _ids.remove(id);
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
            script->setExecQueued(false);

            // check if script needs to be initialized
            if (!(script->getInitialized())) {
                script->runInit();
                script->setInitialized(true);
            }

            // check if script hasn't been killed yet
            if (!(script->getKilled()))
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
            script->setKillQueued(false);

            // check if script needs to be killed
            if (!(script->getKilled())) {
                script->runKill();
                script->setKilled(true);
            }
        }

        _runkillqueue.pop();
    }
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptManager::ScriptManager(unsigned maxcount) :
    _executor(nullptr),
    _maxcount(maxcount)
{
    for (unsigned i = 0; i < maxcount; i++) {
        _scripts.push_back(std::unique_ptr<Script>(nullptr));
        _scriptvalues.push_back(ScriptValues{-1, NULL, nullptr});
    }
}
ScriptManager::~ScriptManager() {}

void ScriptManager::_scriptSetup(Script *script, ScriptType &type, int id) {
    // set up script
    if (type._force_scriptsetup)
        if (_executor)
            script->scriptSetup(_executor);
    script->setType(type._internal_type);

    // set up internal fields
    script->_scriptmanager = this;
    script->_scriptmanager_id = id;
    script->_scriptmanager_removeonkill = type._force_removeonkill;
    script->_scriptmanager_ready = true;

    // enqueue if set
    if (type._force_enqueue)
        script->enqueue();
}

void ScriptManager::_scriptRemoval(ScriptValues &values) {
    if (values._script_ref)
        values._script_ref->scriptResetExec();
}

bool ScriptManager::hasScript(const char *scriptname) { return !(_scripttypes.find(scriptname) == _scripttypes.end()); }

int ScriptManager::spawnScript(const char *scriptname) {
    // fail if exceeding max size
    if (_ids.fillSize() >= _maxcount) {
        std::cerr << "WARN: limit reached in ScriptManager " << this << std::endl;
        return -1;
    }

    // get type information
    ScriptType &type = _scripttypes[scriptname];

    // push to internal storage
    Script *script = type._allocator();
    int id = _ids.push();
    _scripts[id] = std::unique_ptr<Script>(script);
    _scriptvalues[id] = ScriptValues{id, scriptname, script};

    // set up script internals
    _scriptSetup(script, type, id);
    
    return id;
}

Script *ScriptManager::getScript(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._script_ref;
    else
        return nullptr;
}

std::string ScriptManager::getName(int id) {
    if (id >= 0 && _ids.at(id))
        return _scriptvalues[id]._manager_name;
    return "";
}

void ScriptManager::remove(int id) {
    if (id < 0) {
        std::cerr << "WARN: attempt to remove negative value from Manager " << this << std::endl;
        return;
    }
    if (_ids.empty()) {
        std::cerr << "WARN: attempt to remove value from empty Manager " << this << std::endl;
        return;
    }

    if (_ids.at(id)) {
        // get info
        ScriptValues &values = _scriptvalues[id];

        // remove from script-related systems
        _scriptRemoval(values);    
    
        _ids.remove(id);
    }
}

void ScriptManager::addScript(std::function<Script*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill) {
    if (!hasScript(name))
        _scripttypes[name] = ScriptType{
            type,
            force_scriptsetup,
            force_enqueue,
            force_removeonkill,
            allocator
        };
}

void ScriptManager::setExecutor(Executor *executor) { if (!_executor) _executor = executor; }

int ScriptManager::getMaxID() { return _ids.size(); }