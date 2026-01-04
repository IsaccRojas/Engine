#include "../include/script.hpp"

Script::Script(Script &&other) { operator=(std::move(other)); }
Script::Script() :
    _executor(nullptr),
    _executor_id(0),
    _spawn_tag(-1),
    _last_execqueue(-1),
    _initialized(false), 
    _killed(false), 
    _exec_enqueued(false), 
    _kill_enqueued(false),
    _script_name(""),
    _keys_count(0)
{}
Script::~Script() { /* automatic destruction is fine */ }

Script &Script::operator=(Script &&other) {
    if (this != &other) {
        _executor = other._executor;
        _executor_id = other._executor_id;
        _this_iter = other._this_iter;
        _spawn_tag = other._spawn_tag;
        _last_execqueue = other._last_execqueue;
        _initialized = other._initialized;
        _killed = other._killed;
        _exec_enqueued = other._exec_enqueued;
        _kill_enqueued = other._exec_enqueued;
        _script_name = other._script_name;
        _keys = other._keys;
        _keys_count = other._keys_count;
        other._executor = nullptr;
        other._executor_id = 0;
        other._spawn_tag = -1;
        other._last_execqueue = -1;
        other._initialized = false;
        other._killed = false;
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
void Script::runBase() {
    if (_executor)
        _base();
}

void Script::runKill() {
    if (_executor)
        _kill();
}

int Script::getLastExecQueue() { return _last_execqueue; }
bool Script::getInitialized() { return _initialized; }
bool Script::getKilled() { return _killed; }
bool Script::getExecEnqueued() { return _exec_enqueued; }
bool Script::getKillEnqueued() { return _kill_enqueued; }
const char *Script::getName() { return _script_name.c_str(); }

void Script::enqueueExec(unsigned queue) {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for execution with null Executor owner");

    _executor->enqueueExec(_executor_id, queue);
}

void Script::enqueueKill() {
    if (!_executor)
        throw std::runtime_error("Attempt to enqueue for kill with null Executor owner");

    _executor->enqueueKill(_executor_id);
}

unsigned Script::getExecutorID() { return _executor_id; }
int Script::getSpawnTag() { return _spawn_tag; }

void Script::lockout(ScriptKey *k) {
    _keys.insert(k);
    _keys_count++;
}
void Script::unlock(ScriptKey *k) {
    _keys.erase(k);
    _keys_count--;
}
unsigned Script::lockout_count() {
    return _keys_count;
}

// --------------------------------------------------------------------------------------------------------------------------

unsigned Executor::ScriptEnqueue::spawn() {
    return _executor->spawnScript(_name.c_str(), _execution_queue, _tag);
}
Executor::ScriptEnqueue::ScriptEnqueue(Executor *executor, std::string name, int execution_queue, int tag) :
    _executor(executor), _name(name), _execution_queue(execution_queue), _tag(tag)
{}
Executor::ScriptEnqueue::~ScriptEnqueue() { /* automatic destruction is fine */ }

Executor::Executor(unsigned queues) { init(queues); }
Executor::Executor() : _initialized(false) {}
Executor::Executor(Executor &&other) { operator=(std::move(other)); }
Executor::~Executor() { /* automatic destruction is fine */ }

Executor &Executor::operator=(Executor &&other) {
    if (this != &other) {
        std::queue<Script*> empty1;
        std::queue<Script*> empty2;

        _scripts = std::move(other._scripts);
        _intgen = other._intgen;
        _scripts_id = other._scripts_id;
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

void Executor::_setupScript(Script *script, const char *script_name, int execution_queue, int tag) {
    // get information
    ScriptInfo &info = _scriptinfos[script_name];

    // store data
    script->_executor = this;
    script->_executor_id = _intgen.push();
    script->_this_iter = _scripts.push_back(script);
    script->_spawn_tag = tag;

    // store script in map with ID for future referencing
    _scripts_id[script->_executor_id] = script;

    // set script fields (make copy of string passed)
    script->_script_name = script_name;
    
    // enqueue if non-negative queue provided
    if (execution_queue >= 0)
        enqueueExec(script->_executor_id, execution_queue);
    
    // try spawn callback if it exists
    if (info._spawn_callback)
        info._spawn_callback(script);
}

void Executor::_pushSpawnEnqueue(ScriptEnqueue *enqueue) {
    _scriptenqueues.push(enqueue);
}

// TODO: check if ID is valid
void Executor::_checkOwned(Script *script) {
    if (script->_executor != this)
        std::runtime_error("Attempt to use Script reference that is not contained by this Executor");
}

void Executor::_erase(unsigned id) {
    Script *script = _scripts_id[id];
    _checkOwned(script);

    // get values and info
    ScriptInfo &scriptinfo = _scriptinfos[script->_script_name];

    // try removal callback if it exists
    if (scriptinfo._remove_callback)
        scriptinfo._remove_callback(script);

    _intgen.remove(script->_executor_id);
    _scripts_id[script->_executor_id] = nullptr;

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
    _intgen.clear();
    _scripts_id.clear();
    _scriptinfos.clear();
    _scriptenqueues.clear();
    _queuepairs.clear();
    _push_killqueue.swap(empty1);
    _run_killqueue.swap(empty2);
}

void Executor::add(AllocatorInterface *allocator, const char *name, std::function<void(Script*)> spawn_callback, std::function<void(Script*)> remove_callback) {  
    if (!hasAdded(name))
        _scriptinfos[name] = ScriptInfo{allocator, spawn_callback, remove_callback};
    else
        throw std::runtime_error("Attempt to add already added Script name");
}

unsigned Executor::spawnScript(const char *script_name, int execution_queue, int tag) {
    // allocate instance and set it up
    Script *script = _scriptinfos[script_name]._allocator->_allocate(tag);
    _setupScript(script, script_name, execution_queue, tag);
    return script->getExecutorID();
}

void Executor::enqueueSpawn(const char *script_name, int execution_queue, int tag) {
    _pushSpawnEnqueue(new ScriptEnqueue(this, script_name, execution_queue, tag));
}

void Executor::enqueueExec(unsigned id, unsigned queue) {
    Script *script = _scripts_id[id];
    _checkOwned(script);

    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to specified pair
        _queuepairs[queue]._push_execqueue.push(script);
        script->_exec_enqueued = true;
    }
}

void Executor::enqueueKill(unsigned id) {
    Script *script = _scripts_id[id];
    _checkOwned(script);

    if (!(script->_exec_enqueued || script->_kill_enqueued)) {
        // push to kill queue
        _push_killqueue.push(script);
        script->_kill_enqueued = true;
    }
}

std::vector<unsigned> Executor::runSpawnQueue() {
    std::vector<unsigned> scripts;

    while (!(_scriptenqueues.empty())) {
        ScriptEnqueue *scriptenqueue = _scriptenqueues.front();
        scripts.push_back(scriptenqueue->spawn());
        _scriptenqueues.pop();
    }

    return scripts;
}

void Executor::runExecQueue(unsigned queue) {
    // check bounds
    if (queue >= _queuepairs.size())
        throw std::out_of_range("Execution queue index out of range");
    
    std::queue<Script*> &push_execqueue = _queuepairs[queue]._push_execqueue;
    std::queue<Script*> &run_execqueue = _queuepairs[queue]._run_execqueue;
    
    // swap queues
    run_execqueue.swap(push_execqueue);

    Script *script;
    while (!(run_execqueue.empty())) {
        script = run_execqueue.front();
        _checkOwned(script);

        script->_last_execqueue = queue;
        script->_exec_enqueued = false;

        // check if script hasn't been killed yet or kill enqueued
        if (!(script->_killed)) {
            // check if script needs to be initialized
            if (!(script->_initialized)) {
                script->runInit();
                script->_initialized = true;
            }

            script->runBase();
        }
        
        run_execqueue.pop();
    }
}

void Executor::runKillQueue() {
    // swap queues
    _run_killqueue.swap(_push_killqueue);

    Script *script;
    while (!(_run_killqueue.empty())) {
        script = _run_killqueue.front();
        _checkOwned(script);

        script->_kill_enqueued = false;

        // check if script can be killed
        if (script->lockout_count() == 0) {
            // check if script needs to be killed
            if (!(script->_killed)) {
                script->runKill();
                script->_killed = true;

                // remove the script after killing it
                _erase(script->_executor_id);
            }
            
        } else {
            // put script back into queue
            _push_killqueue.push(script);
        }

        _run_killqueue.pop();
    }
}

bool Executor::hasAdded(const char *scriptname) { return !(_scriptinfos.find(scriptname) == _scriptinfos.end()); }

unsigned Executor::getCount() { return _scripts.size(); }

int Executor::getQueueCount() { return _queuepairs.size(); }

bool Executor::initialized() { return _initialized; }