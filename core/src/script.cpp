#include "../include/script.hpp"

ScriptKey::ScriptKey() {}

// --------------------------------------------------------------------------------------------------------------------------

ScriptInterface::ScriptInterface(ScriptInterface&& other) { operator=(std::move(other)); }
ScriptInterface::ScriptInterface() :
    _executor(nullptr),
    _scriptallocator(nullptr),
    _last_execqueue(-1),
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
        _last_execqueue = other._last_execqueue;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._kill_enqueued;
        _kill_started = other._kill_started;
        _script_name = other._script_name;
        _keys = other._keys;
        _keys_count = other._keys_count;
        other._executor = nullptr;
        other._last_execqueue = -1;
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
}

void ScriptInterface::runKill() {
    if (_executor)
        _kill();
}

void ScriptInterface::runUpdate() {
    if (_executor)
        _update();
}

int ScriptInterface::getLastExecQueue() { return _last_execqueue; }
bool ScriptInterface::getExecEnqueued() { return _exec_enqueued; }
bool ScriptInterface::getKillEnqueued() { return _kill_enqueued; }
bool ScriptInterface::getKillStarted() { return _kill_started; }
const char *ScriptInterface::getName() { return _script_name.c_str(); }

void ScriptInterface::enqueueExec(unsigned queue) {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for execution with null ScriptExecutor owner");

    _executor->enqueueExec(ScriptView(this), queue);
}

void ScriptInterface::enqueueKill() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for kill with null ScriptExecutor owner");

    _executor->enqueueKill(ScriptView(this));
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

ScriptView::ScriptView(ScriptInterface* script) : _script(script) {};

void ScriptView::enqueueExec(unsigned queue) { _script->enqueueExec(queue); }
void ScriptView::enqueueKill() { _script->enqueueKill(); }
int ScriptView::getLastExecQueue() { return _script->getLastExecQueue(); }
bool ScriptView::getExecEnqueued() { return _script->getExecEnqueued(); }
bool ScriptView::getKillEnqueued() { return _script->getKillEnqueued(); }
const char* ScriptView::getName() { return _script->getName(); }
ScriptKey ScriptView::key() { return _script->key(); }
void ScriptView::lockout(ScriptKey *k) { _script->lockout(k); }
void ScriptView::unlock(ScriptKey *k) { _script->unlock(k); }
ScriptInterface* ScriptView::getScript() { return _script; }

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

ScriptExecutor::ScriptEnqueue::ScriptEnqueue(ScriptExecutor* executor, std::string name, int execution_queue) :
    _executor(executor), _name(name), _execution_queue(execution_queue)
{}
ScriptExecutor::ScriptEnqueue::~ScriptEnqueue() { /* automatic destruction is fine */ }
ScriptView ScriptExecutor::ScriptEnqueue::spawn() {
    return _executor->spawnScript(_name.c_str(), _execution_queue);
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptExecutor::ScriptExecutor(unsigned queues) { init(queues); }
ScriptExecutor::ScriptExecutor() : _initialized(false) {}
ScriptExecutor::ScriptExecutor(ScriptExecutor&& other) { operator=(std::move(other)); }
ScriptExecutor::~ScriptExecutor() { /* automatic destruction is fine */ }

ScriptExecutor &ScriptExecutor::operator=(ScriptExecutor&& other) {
    if (this != &other) {
        std::queue<ScriptInterface*> empty1;
        std::queue<ScriptInterface*> empty2;

        _scripts = std::move(other._scripts);
        _scriptinfos = other._scriptinfos;
        _scriptenqueues = std::move(other._scriptenqueues);
        _queuepairs = other._queuepairs;
        _push_killqueue = other._push_killqueue;
        _run_killqueue = other._run_killqueue;

        // safe as structures owning memory are already moved
        other.uninit();
    }
    return *this;
}

void ScriptExecutor::_setupScript(ScriptInterface* script, const char* script_name, int execution_queue, ScriptAllocatorInterface* scriptallocator) {
    // get information
    ScriptInfo &info = _scriptinfos[script_name];

    // store data
    script->_executor = this;
    script->_scriptallocator = scriptallocator;
    script->_this_iter = _scripts.push_back(script);

    // store in allocator
    scriptallocator->_insertReference(script);

    // set script fields (make copy of string passed)
    script->_script_name = script_name;
    
    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(ScriptView(script), execution_queue);
    
    // try spawn callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(script);
}

void ScriptExecutor::_pushSpawnEnqueue(ScriptEnqueue *enqueue) {
    _scriptenqueues.push(enqueue);
}

void ScriptExecutor::_erase(ScriptInterface* script) {
    // get values and info
    ScriptInfo &scriptinfo = _scriptinfos[script->_script_name];

    // try removal callback if it exists
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(script);
    
    scriptinfo._allocator->_onDeallocation(script);

    script->_scriptallocator->_removeReference(script);
    _scripts.erase(script->_this_iter);
}


void ScriptExecutor::init(unsigned queues) {
    if (_initialized)
        throw InitializedException();
    
    _queuepairs = std::vector<QueuePair>(queues, QueuePair{});
    _initialized = true;
}

void ScriptExecutor::uninit() {
    if (!_initialized)
        return;

    std::queue<ScriptInterface*> empty1;
    std::queue<ScriptInterface*> empty2;

    _scripts.clear();
    _scriptinfos.clear();
    _scriptenqueues.clear();
    _queuepairs.clear();
    _push_killqueue.swap(empty1);
    _run_killqueue.swap(empty2);
    _initialized = false;
}

void ScriptExecutor::addScript(ScriptInfo scriptinfo, const char* name) {  
    if (!hasAdded(name))
        _scriptinfos[name] = scriptinfo;
    else
        throw std::runtime_error("Attempt to add already added ScriptInterface name");
}

ScriptView ScriptExecutor::spawnScript(const char* script_name, int execution_queue) {
    // allocate instance and set it up
    ScriptInterface* script = _scriptinfos[script_name]._allocator->_allocate();
    _setupScript(script, script_name, execution_queue, _scriptinfos[script_name]._allocator);

    // run initialization method
    script->runInit();

    return ScriptView(script);
}

void ScriptExecutor::enqueueSpawn(const char* script_name, int execution_queue) {
    _pushSpawnEnqueue(new ScriptEnqueue(this, script_name, execution_queue));
}

void ScriptExecutor::enqueueExec(ScriptView scriptview, unsigned queue) {
    ScriptInterface* script = scriptview.getScript();

    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to specified pair
        _queuepairs[queue]._push_execqueue.push(script);
        script->_exec_enqueued = true;
    }
}

void ScriptExecutor::enqueueKill(ScriptView scriptview) {
    ScriptInterface* script = scriptview.getScript();

    if (!(script->_kill_enqueued)) {
        // push to kill queue
        _push_killqueue.push(script);
        script->_kill_enqueued = true;
    }
}

std::vector<ScriptView> ScriptExecutor::runSpawnQueue() {
    std::vector<ScriptView> scriptviews;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue* scriptenqueue = _scriptenqueues.front();
        scriptviews.push_back(scriptenqueue->spawn());
        _scriptenqueues.pop();
    }

    return scriptviews;
}

void ScriptExecutor::runExecQueue(unsigned queue) {
    // check bounds
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");
    
    std::queue<ScriptInterface*>& push_execqueue = _queuepairs[queue]._push_execqueue;
    std::queue<ScriptInterface*>& run_execqueue = _queuepairs[queue]._run_execqueue;
    
    // swap queues
    run_execqueue.swap(push_execqueue);

    ScriptInterface* script;
    while (!(run_execqueue.empty())) {
        script = run_execqueue.front();

        script->_last_execqueue = queue;
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

bool ScriptExecutor::initialized() { return _initialized; }