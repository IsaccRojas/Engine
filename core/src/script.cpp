#include "../include/script.hpp"

ScriptKey::ScriptKey() {}

// --------------------------------------------------------------------------------------------------------------------------

ScriptInterface::ScriptInterface(ScriptInterface&& other) { operator=(std::move(other)); }
ScriptInterface::ScriptInterface() :
    _executor(nullptr),
    _scriptallocator(nullptr),
    _preferred_queue(-1),
    _auto_enqueue(false),
    _exec_enqueued(false), 
    _kill_enqueued(false),
    _kill_started(false),
    _script_name(""),
    _keys_count(0)
{}
ScriptInterface::~ScriptInterface() {}

ScriptInterface& ScriptInterface::operator=(ScriptInterface&& other) {
    if (this != &other) {
        _executor = other._executor;
        _this_iter = other._this_iter;
        _preferred_queue = other._preferred_queue;
        _auto_enqueue = other._auto_enqueue;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._kill_enqueued;
        _kill_started = other._kill_started;
        _script_name = other._script_name;
        _keys = other._keys;
        _keys_count = other._keys_count;
        other._executor = nullptr;
        other._preferred_queue = -1;
        other._auto_enqueue = false;
        other._exec_enqueued = false;
        other._exec_enqueued = false;
        other._kill_started = false;
        other._script_name = "";
        other._keys.clear();
        other._keys_count = 0;
    }
    return *this;
}

void ScriptInterface::runInit() {
    if (_executor)
        _init();

}
void ScriptInterface::runExec() {
    if (_executor)
        _exec();
    enqueueExec();
}

void ScriptInterface::runKill() {
    if (_executor)
        _kill();
}

void ScriptInterface::runUpdate() {
    if (_executor)
        _update();
}

int& ScriptInterface::preferred_queue() { return _preferred_queue; }
bool& ScriptInterface::auto_enqueue() { return _auto_enqueue; }
bool ScriptInterface::getExecEnqueued() { return _exec_enqueued; }
bool ScriptInterface::getKillEnqueued() { return _kill_enqueued; }
bool ScriptInterface::getKillStarted() { return _kill_started; }
const char *ScriptInterface::getName() { return _script_name.c_str(); }

void ScriptInterface::enqueueExec() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for execution with null ScriptExecutor owner");
    if (_preferred_queue < 0)
        throw std::runtime_error("Attempt to enqueue for execution through Script with negative preferred queue");

    _executor->enqueueExec(this, _preferred_queue);
}

void ScriptInterface::enqueueKill() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for kill with null ScriptExecutor owner");

    _executor->enqueueKill(this);
}

ScriptKey& ScriptInterface::key() { return _key; };

void ScriptInterface::lockout(ScriptKey* k) {
    _keys.insert(k);
}

void ScriptInterface::unlock(ScriptKey* k) {
    _keys.erase(k);
}

unsigned ScriptInterface::lockoutCount() {
    return _keys.size();
}

// --------------------------------------------------------------------------------------------------------------------------

void ScriptAllocatorInterface::_insertReference(ScriptInterface* script) {
    _scripts.insert(script);
}

void ScriptAllocatorInterface::_removeReference(ScriptInterface* script) {
    _scripts.erase(script);
}

ScriptAllocatorInterface::ScriptAllocatorInterface() {}

ScriptAllocatorInterface::~ScriptAllocatorInterface() {}

bool ScriptAllocatorInterface::hasReference(ScriptInterface* script) {
    return (_scripts.find(script) != _scripts.end());
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptExecutor::ScriptEnqueue::ScriptEnqueue(ScriptExecutor* executor, std::string name) :
    _executor(executor), _name(name)
{}
ScriptExecutor::ScriptEnqueue::~ScriptEnqueue() { /* automatic destruction is fine */ }
ScriptInterface* ScriptExecutor::ScriptEnqueue::spawn() {
    return _executor->spawnScript(_name.c_str());
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptExecutor::ScriptExecutor(unsigned queues) {
    _queuepairs = std::vector<QueuePair>(queues, QueuePair{});
}
ScriptExecutor::ScriptExecutor(ScriptExecutor&& other) { operator=(std::move(other)); }
ScriptExecutor::~ScriptExecutor() { /* default destruction is fine */ }

ScriptExecutor &ScriptExecutor::operator=(ScriptExecutor&& other) {
    if (this != &other) {
        _scripts = std::move(other._scripts);
        _scriptinfos = other._scriptinfos;
        _scriptenqueues = std::move(other._scriptenqueues);
        _queuepairs = other._queuepairs;
        _push_killqueue = other._push_killqueue;
        _run_killqueue = other._run_killqueue;

        other._scripts.clear();
        other._scriptinfos.clear();
        other._scriptenqueues.clear();
        other._queuepairs.clear();
    
        std::queue<ScriptInterface*> empty1;
        std::queue<ScriptInterface*> empty2;
        other._push_killqueue.swap(empty1);
        other._run_killqueue.swap(empty2);
    }

    return *this;
}

void ScriptExecutor::_setupScript(ScriptInterface* script, const char* script_name, ScriptAllocatorInterface* scriptallocator) {
    // get information
    ScriptInfo &info = _scriptinfos[script_name];

    // store data
    script->_executor = this;
    script->_scriptallocator = scriptallocator;
    script->_this_iter = _scripts.push_back(script);
    script->_preferred_queue = info.preferred_queue;
    script->_auto_enqueue = info.auto_enqueue;
    script->_script_name = script_name;

    // store in allocator
    scriptallocator->_insertReference(script);

    // enqueue if auto-enqueue set, and non-negative queue provided
    if (script->auto_enqueue() && script->preferred_queue() >= 0)
        enqueueExec(script, script->preferred_queue());
    
    // try spawn callback if it exists
    if (info.spawn_callback)
        info.spawn_callback(script);
}

void ScriptExecutor::_pushSpawnEnqueue(ScriptEnqueue *enqueue) {
    _scriptenqueues.push(enqueue);
}

void ScriptExecutor::_erase(ScriptInterface* script) {
    // get values and info
    ScriptInfo &scriptinfo = _scriptinfos[script->_script_name];

    // try removal callback if it exists
    if (scriptinfo.remove_callback)
        scriptinfo.remove_callback(script);
    
    scriptinfo.allocator->_onDeallocation(script);

    script->_scriptallocator->_removeReference(script);
    _scripts.erase(script->_this_iter);
}

void ScriptExecutor::addScript(ScriptInfo scriptinfo, const char* name) {  
    if (hasAdded(name))
        throw std::runtime_error((std::string("Attempt to add existing ScriptInfo name '") + name) + std::string("'"));
    _scriptinfos[name] = scriptinfo;
        
}

ScriptInterface* ScriptExecutor::spawnScript(const char* script_name) {
    auto si_iter = _scriptinfos.find(script_name);
    if (si_iter == _scriptinfos.end())
        throw std::runtime_error((std::string("Attempt to spawn Script with non-existent ScriptInfo name '") + script_name) + std::string("'"));

    // allocate instance and set it up
    ScriptInterface* script = si_iter->second.allocator->_allocate();
    _setupScript(script, script_name, si_iter->second.allocator);

    // run initialization method
    script->runInit();

    return script;
}

void ScriptExecutor::enqueueSpawn(const char* script_name) {
    _pushSpawnEnqueue(new ScriptEnqueue(this, script_name));
}

void ScriptExecutor::enqueueExec(ScriptInterface* script, unsigned queue) {
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Attempt to enqueue into ScriptExecutor for execution with queue index out of range");

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to specified pair
        _queuepairs[queue]._push_execqueue.push(script);
        script->_exec_enqueued = true;
    }
}

void ScriptExecutor::enqueueKill(ScriptInterface* script) {
    if (!(script->_kill_enqueued)) {
        // push to kill queue
        _push_killqueue.push(script);
        script->_kill_enqueued = true;
    }
}

std::vector<ScriptInterface*> ScriptExecutor::runSpawnQueue() {
    std::vector<ScriptInterface*> scripts;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue* scriptenqueue = _scriptenqueues.front();
        scripts.push_back(scriptenqueue->spawn());
        _scriptenqueues.pop();
    }

    return scripts;
}

void ScriptExecutor::runExecQueue(unsigned queue) {
    // check bounds
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Attempt to run execution queue with index out of range");
    
    std::queue<ScriptInterface*>& push_execqueue = _queuepairs[queue]._push_execqueue;
    std::queue<ScriptInterface*>& run_execqueue = _queuepairs[queue]._run_execqueue;
    
    // swap queues
    run_execqueue.swap(push_execqueue);

    ScriptInterface* script;
    while (!(run_execqueue.empty())) {
        script = run_execqueue.front();
        
        script->_exec_enqueued = false;
        script->runExec();
        
        run_execqueue.pop();
    }
}

void ScriptExecutor::runKillQueue() {
    // swap queues
    _run_killqueue.swap(_push_killqueue);

    ScriptInterface* script;
    while (!(_run_killqueue.empty())) {
        script = _run_killqueue.front();

        // check if script can be killed
        if (script->lockoutCount() == 0) {
            script->_kill_started = true;
            script->runKill();

            // remove the script after killing it
            _erase(script);
        } else {
            // put script back into queue
            _push_killqueue.push(script);
        }

        _run_killqueue.pop();
    }
}

void ScriptExecutor::runUpdate() {
    for (auto iter = _scripts.begin(); iter != _scripts.end(); ++iter)
        (*iter)->runUpdate();
}

bool ScriptExecutor::hasAdded(const char* scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

unsigned ScriptExecutor::getCount() { return _scripts.size(); }

int ScriptExecutor::getQueueCount() { return _queuepairs.size(); }