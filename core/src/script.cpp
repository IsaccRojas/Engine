#include "../include/script.hpp"

ScriptKey::ScriptKey() {}

// --------------------------------------------------------------------------------------------------------------------------

Script::Script(Script&& other) { operator=(std::move(other)); }
Script::Script() :
    _executor(nullptr),
    _last_execqueue(-1),
    _exec_enqueued(false), 
    _kill_enqueued(false),
    _script_name(""),
    _keys_count(0)
{}
Script::~Script() { /* automatic destruction is fine */ }

Script& Script::operator=(Script&& other) {
    if (this != &other) {
        _executor = other._executor;
        _this_iter = other._this_iter;
        _last_execqueue = other._last_execqueue;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._kill_enqueued;
        _script_name = other._script_name;
        _keys = other._keys;
        _keys_count = other._keys_count;
        other._executor = nullptr;
        other._last_execqueue = -1;
        other._exec_enqueued = false;
        other._exec_enqueued = false;
        other._script_name = "";
        other._keys.clear();
        other._keys_count = 0;
    }
    return *this;
}

void Script::runInit() {
    if (_executor)
        _init();

}
void Script::runExec() {
    if (_executor)
        _exec();
}

void Script::runKill() {
    if (_executor)
        _kill();
}

void Script::runUpdate() {
    if (_executor)
        _update();
}

int Script::getLastExecQueue() { return _last_execqueue; }
bool Script::getExecEnqueued() { return _exec_enqueued; }
bool Script::getKillEnqueued() { return _kill_enqueued; }
const char *Script::getName() { return _script_name.c_str(); }

void Script::enqueueExec(unsigned queue) {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for execution with null Executor owner");

    _executor->enqueueExec(ScriptView(this), queue);
}

void Script::enqueueKill() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for kill with null Executor owner");

    _executor->enqueueKill(ScriptView(this));
}

ScriptKey& Script::key() { return _key; };
void Script::lockout(ScriptKey* k) {
    _keys.insert(k);
}
void Script::unlock(ScriptKey* k) {
    _keys.erase(k);
}
unsigned Script::lockout_count() {
    return _keys.size();
}

// --------------------------------------------------------------------------------------------------------------------------

ScriptView::ScriptView(Script* script) : _script(script) {};

void ScriptView::enqueueExec(unsigned queue) { _script->enqueueExec(queue); }
void ScriptView::enqueueKill() { _script->enqueueKill(); }
int ScriptView::getLastExecQueue() { return _script->getLastExecQueue(); }
bool ScriptView::getExecEnqueued() { return _script->getExecEnqueued(); }
bool ScriptView::getKillEnqueued() { return _script->getKillEnqueued(); }
const char* ScriptView::getName() { return _script->getName(); }
ScriptKey ScriptView::key() { return _script->key(); }
void ScriptView::lockout(ScriptKey *k) { _script->lockout(k); }
void ScriptView::unlock(ScriptKey *k) { _script->unlock(k); }

// --------------------------------------------------------------------------------------------------------------------------

Executor::ScriptEnqueue::ScriptEnqueue(Executor* executor, std::string name, int execution_queue) :
    _executor(executor), _name(name), _execution_queue(execution_queue)
{}
Executor::ScriptEnqueue::~ScriptEnqueue() { /* automatic destruction is fine */ }
ScriptView Executor::ScriptEnqueue::spawn() {
    return _executor->spawnScript(_name.c_str(), _execution_queue);
}

// --------------------------------------------------------------------------------------------------------------------------

Executor::Executor(unsigned queues) { init(queues); }
Executor::Executor() : _initialized(false) {}
Executor::Executor(Executor&& other) { operator=(std::move(other)); }
Executor::~Executor() { /* automatic destruction is fine */ }

Executor &Executor::operator=(Executor&& other) {
    if (this != &other) {
        std::queue<Script*> empty1;
        std::queue<Script*> empty2;

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

void Executor::_setupScript(Script* script, const char* script_name, int execution_queue) {
    // get information
    ScriptInfo &info = _scriptinfos[script_name];

    // store data
    script->_executor = this;
    script->_this_iter = _scripts.push_back(script);

    // set script fields (make copy of string passed)
    script->_script_name = script_name;
    
    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(ScriptView(script), execution_queue);
    
    // try spawn callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(script);
}

void Executor::_pushSpawnEnqueue(ScriptEnqueue *enqueue) {
    _scriptenqueues.push(enqueue);
}

void Executor::_erase(Script* script) {
    // get values and info
    ScriptInfo &scriptinfo = _scriptinfos[script->_script_name];

    // try removal callback if it exists
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(script);

    _scripts.erase(script->_this_iter);
}


void Executor::init(unsigned queues) {
    if (_initialized)
        throw InitializedException();
    
    _queuepairs = std::vector<QueuePair>(queues, QueuePair{});
    _initialized = true;
}

void Executor::uninit() {
    if (!_initialized)
        return;

    std::queue<Script*> empty1;
    std::queue<Script*> empty2;

    _scripts.clear();
    _scriptinfos.clear();
    _scriptenqueues.clear();
    _queuepairs.clear();
    _push_killqueue.swap(empty1);
    _run_killqueue.swap(empty2);
    _initialized = false;
}

void Executor::addScript(ScriptInfo scriptinfo, const char* name) {  
    if (!hasAdded(name))
        _scriptinfos[name] = scriptinfo;
    else
        throw std::runtime_error("Attempt to add already added Script name");
}

ScriptView Executor::spawnScript(const char* script_name, int execution_queue) {
    // allocate instance and set it up
    Script* script = _scriptinfos[script_name]._allocator->_allocate();
    _setupScript(script, script_name, execution_queue);

    // run initialization method
    script->runInit();

    return ScriptView(script);
}

void Executor::enqueueSpawn(const char* script_name, int execution_queue) {
    _pushSpawnEnqueue(new ScriptEnqueue(this, script_name, execution_queue));
}

void Executor::enqueueExec(ScriptView scriptview, unsigned queue) {
    Script* script = scriptview._script;

    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to specified pair
        _queuepairs[queue]._push_execqueue.push(script);
        script->_exec_enqueued = true;
    }
}

void Executor::enqueueKill(ScriptView scriptview) {
    // TODO: check if ID is valid
    Script* script = scriptview._script;

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to kill queue
        _push_killqueue.push(script);
        script->_kill_enqueued = true;
    }
}

std::vector<ScriptView> Executor::runSpawnQueue() {
    std::vector<ScriptView> scriptviews;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue* scriptenqueue = _scriptenqueues.front();
        scriptviews.push_back(scriptenqueue->spawn());
        _scriptenqueues.pop();
    }

    return scriptviews;
}

void Executor::runExecQueue(unsigned queue) {
    // check bounds
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");
    
    std::queue<Script*>& push_execqueue = _queuepairs[queue]._push_execqueue;
    std::queue<Script*>& run_execqueue = _queuepairs[queue]._run_execqueue;
    
    // swap queues
    run_execqueue.swap(push_execqueue);

    Script* script;
    while (!(run_execqueue.empty())) {
        script = run_execqueue.front();

        script->_last_execqueue = queue;
        script->_exec_enqueued = false;
        script->runExec();
        
        run_execqueue.pop();
    }
}

void Executor::runKillQueue() {
    // swap queues
    _run_killqueue.swap(_push_killqueue);

    Script* script;
    while (!(_run_killqueue.empty())) {
        script = _run_killqueue.front();

        // check if script can be killed
        if (script->lockout_count() == 0) {
            script->runKill();
            script->_kill_enqueued = false;

            // remove the script after killing it
            _erase(script);
        } else {
            // put script back into queue
            _push_killqueue.push(script);
        }

        _run_killqueue.pop();
    }
}

void Executor::runUpdate() {
    for (auto iter = _scripts.begin(); iter != _scripts.end(); ++iter)
        (*iter)->runUpdate();
}

bool Executor::hasAdded(const char* scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

unsigned Executor::getCount() { return _scripts.size(); }

int Executor::getQueueCount() { return _queuepairs.size(); }

bool Executor::initialized() { return _initialized; }