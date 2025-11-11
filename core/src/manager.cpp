#include "../include/manager.hpp"

Manager::Manager(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box) : _initialized(false) { init(executor, glenv, physspace_box); }
Manager::Manager() : _executor(nullptr), _glenv(nullptr), _physspace_box(nullptr), _initialized(false) {}

void Manager::init(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box) {
    if (_initialized)
        throw InitializedException();
    
    _executor = executor;
    _glenv = glenv;
    _physspace_box = physspace_box;
    _initialized = true;
}

void Manager::uninit() {
    if (!_initialized)
        return;
    
    _executor = nullptr;
    _glenv = nullptr;
    _physspace_box = nullptr;
    _schemes.clear();
    _initialized = false;
}

void Manager::addScheme(Scheme s, const char *name) {
    _schemes[name] = s;
}

void Manager::instScheme(const char *name) {
    Scheme &s = _schemes[name];

    for (const ScriptArgs &a : s._script_args)
        _executor->spawnScript(a.script_name, a.execution_queue, a.tag);
    for (const QuadArgs &a : s._quad_args)
        _glenv->genQuad(a.pos, a.scale, a.color, a.type, a.animation_name, a.texpos, a.texsize, a.innerrad);
    for (const BoxArgs &a : s._box_args)
        _physspace_box->push(a.transf, a.vel, a.callback, a.filter_name);
}