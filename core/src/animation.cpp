#include "../include/animation.hpp"

Frame::Frame(glm::vec3 textureposition, glm::vec2 texturesize, unsigned frameduration) :
    texpos(textureposition), texsize(texturesize), duration(frameduration)
{}
Frame::Frame() : duration(0) {}

Cycle::Cycle(const char* name, bool loop, std::vector<Frame> frames) : _name(name), _loop(loop), _frames(frames) {}
Cycle::Cycle() : _name(""), _loop(false) {}
Cycle::~Cycle() { /* automatic destruction is fine */ }

Cycle& Cycle::setName(const char* name) {
    _name = name;
    return *this;
}

Cycle& Cycle::setLoop(bool loop) {
    _loop = loop;
    return *this;
}

Cycle& Cycle::addFrame(const Frame &frame) {
    _frames.push_back(Frame{frame.texpos, frame.texsize, frame.duration});
    return *this;
}

std::string Cycle::name() {
    return _name;
}

bool Cycle::loop() {
    return _loop;
}

Frame& Cycle::frame(unsigned i) {
    return _frames[i];
}

unsigned Cycle::count() const {
    return _frames.size();
}

Animation::Animation(const char* name, std::vector<Cycle> cycles) : _name(name), _cycles(cycles) {
    for (int i = 0; i < _cycles.size(); i++)
        _cycle_indices[_cycles[i].name()] = i;
}
Animation::Animation() {}
Animation::~Animation() { /* automatic destruction is fine */ }

Animation& Animation::setName(const char* name) {
    _name = name;
    return *this;
}
Animation& Animation::addCycle(const Cycle& cycle) {
    _cycles.push_back(cycle);
    return *this;
}

std::string Animation::name() {
    return _name;
}

Cycle& Animation::cycle(unsigned i) {
    return _cycles[i];
}
Cycle& Animation::cycle(const char* name) {
    return _cycles[_cycle_indices[name]];
}

unsigned Animation::cycleIndex(const char* name) {
    return _cycle_indices[name];
}

unsigned Animation::count() {
    return _cycles.size();
}

AnimationState::AnimationState(Animation* animation) {
    setAnimation(animation);
}
AnimationState::AnimationState() :
    _animation(nullptr),
    _current_cycle(nullptr),
    _current_frame(nullptr),
    _step(0),
    _cycle_state(0),
    _frame_state(0),
    _completed(false)
{}

AnimationState::~AnimationState() { /* automatic destruction is fine */ }

void AnimationState::setAnimation(Animation* animation) {
    _animation = animation;
    
    _step = 0;
    _cycle_state = 0;
    _frame_state = 0;
    _completed = false;

    if (_animation) {
        _current_cycle = &(_animation->cycle(_cycle_state));
        _current_frame = &(_current_cycle->frame(_frame_state));
    } else {
        _current_cycle = nullptr;
        _current_frame = nullptr;
    }
}

void AnimationState::setCycleState(unsigned i) {
    if (!_animation)
        throw std::runtime_error("Attempt to set cycle state with null Animation reference");

    if (_cycle_state == i)
        return;

    _cycle_state = i;
    _current_cycle = &(_animation->cycle(i));
    this->setFrameState(0);
}
void AnimationState::setCycleState(const char* name) {
    if (!_animation)
        throw std::runtime_error("Attempt to set cycle state with null Animation reference");

    unsigned i = _animation->cycleIndex(name);

    if (_cycle_state == i)
        return;

    _cycle_state = i;
    _current_cycle = &(_animation->cycle(i));
    this->setFrameState(0);
}

void AnimationState::setFrameState(unsigned i) {
    if (!_animation)
        throw std::runtime_error("Attempt to set frame state with null Animation reference");

    _frame_state = i;
    _current_frame = &(_current_cycle->frame(_frame_state));
    _step = 0;
    _completed = false;
        
}

void AnimationState::step() {
    if (!_animation)
        throw std::runtime_error("Attempt to step with null Animation reference");

    // check if completed
    if (_completed)
        return;
    
    // advance one step in time
    _step++;

    // check if duration of current frame is over
    if (_step >= _current_frame->duration) {
        // check if on last frame of cycle
        if (_frame_state + 1 >= _current_cycle->count()) {
            // check if we should loop
            if (_current_cycle->loop()) {
                this->setFrameState(0);
                return;
            }

            // otherwise, stay here
            _completed = true;
            return;
        }

        // otherwise, go to next frame and reset time
        this->setFrameState(_frame_state + 1);
    }
}

const Frame& AnimationState::current() {
    if (!_animation)
        throw std::runtime_error("Attempt to get current frame with null Animation reference");
    
    return *_current_frame;
}

bool AnimationState::hasAnimation() { return _animation != nullptr; }

bool AnimationState::animationEmpty() { return _animation->count() == 0; }

bool AnimationState::completed() { return _completed; }