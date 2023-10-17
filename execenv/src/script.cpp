#include "script.hpp"

Script::Script() : _initialized(false), _killed(false), _id(-1) {}

void Script::_init() {}
void Script::_base() {}
void Script::_kill() {}

void Script::init() {
    // check if Script has been initialized yet
    if (!_initialized) {
        _init();
        _initialized = true;
    }
}
void Script::base() {
    _base();
}

void Script::kill() {
    // check if Script has been killed yet
    if (!_killed) {
        _kill();
        _killed = true;
    }
}

int Script::id() { return _id; }