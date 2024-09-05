#include "../include/animation.hpp"

Cycle::Cycle(bool loop) {
    setLoop(loop);
}
Cycle::Cycle() : _loop(false) {}
Cycle::~Cycle() { /* automatic destruction is fine */ }

Cycle& Cycle::addFrame(glm::vec3 texpos, glm::vec2 texsize, glm::vec3 offset, int duration) {
    _frames.push_back(Frame{texpos, texsize, offset, duration});
    return *this;
}
Cycle& Cycle::addFrame(const Frame &frame) {
    _frames.push_back(Frame{frame.texpos, frame.texsize, frame.offset, frame.duration});
    return *this;
}

void Cycle::setLoop(bool loop) {
    _loop = loop;
}

Frame& Cycle::getFrame(int i) {
    return _frames[i];
}

int Cycle::count() const {
    return _frames.size();
}

bool Cycle::loops() const {
    return _loop;
}

Animation::Animation() {}
Animation::~Animation() { /* automatic destruction is fine */ }

Animation& Animation::addCycle(Cycle &cycle) {
    _cycles.push_back(cycle);
    return *this;
}

Cycle& Animation::getCycle(int i) {
    return _cycles[i];
}

int Animation::count() {
    return _cycles.size();
}

AnimationState::AnimationState(Animation *animation) {
    setAnimation(animation);
}
AnimationState::AnimationState() :
    _animation(nullptr),
    _current_cycle(nullptr),
    _current_frame(nullptr),
    _step(0), 
    _frame_state(0), 
    _cycle_state(0), 
    _completed(false)
{}

AnimationState::~AnimationState() { /* automatic destruction is fine */ }

void AnimationState::setAnimation(Animation *animation) {
    _animation = animation;

    if (_animation) {
        _current_cycle = &(_animation->getCycle(_cycle_state));
        _current_frame = &(_current_cycle->getFrame(_frame_state));
    } else {
        _current_cycle = nullptr;
        _current_frame = nullptr;
    }

    _step = 0;
    _frame_state = 0;
    _cycle_state = 0;
    _completed = false;
}

void AnimationState::setCycleState(int cyclestate) {
    if (!_animation) {
        std::cerr << "WARN: AnimationState::setCycleState: attempt to set cycle state with null animation reference in AnimationState instance " << this << std::endl;
        return;
    }

    if (cyclestate == _cycle_state)
        return;
    _cycle_state = cyclestate;
    _current_cycle = &(_animation->getCycle(_cycle_state));
    this->setFrameState(0);
}

void AnimationState::setFrameState(int framestate) {
    if (!_animation) {
        std::cerr << "WARN: AnimationState::setFrameState: attempt to set frame state with null animation reference in AnimationState instance " << this << std::endl;
        return;
    }

    _frame_state = framestate;
    _current_frame = &(_current_cycle->getFrame(_frame_state));
    _step = 0;
    _completed = false;
        
}

void AnimationState::step() {
    if (!_animation) {
        std::cerr << "WARN: AnimationState::step: attempt to step animation with null animation reference in AnimationState instance " << this << std::endl;
        return;
    }

    // check if completed or if no frames exist
    if (_completed || _current_cycle->count() == 0)
        return;
    
    // advance one step in time
    _step++;

    // check if duration of current framestate is over
    if (_step >= _current_frame->duration) {
        // check if on last framestate
        if (_frame_state + 1 >= _current_cycle->count()) {
            // check if we should loop
            if (_current_cycle->loops()) {
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

Frame *AnimationState::getCurrent() {
    if (!_animation) {
        std::cerr << "WARN: AnimationState::getCurrent: attempt to get current frame with null animation reference in AnimationState instance " << this << std::endl;
        return nullptr;
    }
    
    return _current_frame;
}

bool AnimationState::completed() {
    return _completed;
}

std::unordered_map<std::string, Animation> loadAnimations(std::string dir) {
    std::unordered_map<std::string, Animation> animations;
    
    // iterate on provided directory
    nlohmann::json data;
    std::string filename;
    for (auto iter = std::filesystem::directory_iterator(dir); iter != std::filesystem::directory_iterator(); iter++) {
        filename = iter->path().string();

        // check if file ends in .json
        if (endsWith(filename, ".json")) {
            
            // try to parse .json file
            try {
                data = nlohmann::json::parse(std::ifstream(filename));
            } catch (const std::exception& e) {
                std::cerr << "Error loading .json file '" << filename << "': " << e.what() << std::endl;
                goto dir_loop_end;
            }

            // variables to store all retrieved fields
            std::string name;
            std::vector<Cycle> cycles;

            // retrieve name
            if (data.contains("name") && data["name"].is_string())
                name = data["name"];
            else {
                std::cerr << "Error loading .json file '" << filename << "': field 'name' is missing or is not a string" << std::endl;
                goto dir_loop_end;
            }

            // retrieve cycles
            if (data.contains("cycles") && data["cycles"].is_object()) {

                // iterate on cycles
                for (nlohmann::json::iterator iter_cycles = data["cycles"].begin(); iter_cycles != data["cycles"].end(); iter_cycles++) {
                    
                    // retrieve cycle
                    if (data["cycles"][iter_cycles.key()].is_object()) {
                        cycles.push_back(Cycle{});

                        // retrieve frames
                        if (data["cycles"][iter_cycles.key()].contains("frames") && data["cycles"][iter_cycles.key()]["frames"].is_object()) {
                            auto frames_data = data["cycles"][iter_cycles.key()]["frames"];

                            // iterate on frames
                            for (nlohmann::json::iterator iter_frames = frames_data.begin(); iter_frames != frames_data.end(); iter_frames++) {
                                
                                // retrieve frame
                                if (frames_data[iter_frames.key()].is_object()) {
                                    auto frame_data = frames_data[iter_frames.key()];
                                    Frame frame;
                                    
                                    // get frame texpos
                                    if (frame_data.contains("texpos") && frame_data["texpos"].is_array() && frame_data["texpos"].size() == 3) {
                                        // get first value
                                        auto value0 = frame_data["texpos"][0];
                                        if (value0.is_number_float()) {
                                            frame.texpos.x = value0;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 0 of field 'texpos' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }

                                        // get second value
                                        auto value1 = frame_data["texpos"][1];
                                        if (value1.is_number_float()) {
                                            frame.texpos.y = value1;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 1 of field 'texpos' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }

                                        // get third value
                                        auto value2 = frame_data["texpos"][2];
                                        if (value2.is_number_float()) {
                                            frame.texpos.z = value2;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 2 of field 'texpos' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }
                                    } else {
                                        std::cerr << "Error loading .json file '" << filename << "': field 'texpos' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' does not exist, is not an array, or is not length 3" << std::endl;
                                        goto dir_loop_end;
                                    }

                                    // get frame texsize
                                    if (frame_data.contains("texsize") && frame_data["texsize"].is_array() && frame_data["texsize"].size() == 2) {
                                        // get first value
                                        auto value0 = frame_data["texsize"][0];
                                        if (value0.is_number_float()) {
                                            frame.texsize.x = value0;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 0 of field 'texsize' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }

                                        // get second value
                                        auto value1 = frame_data["texsize"][1];
                                        if (value1.is_number_float()) {
                                            frame.texsize.y = value1;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 1 of field 'texsize' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }
                                    } else {
                                        std::cerr << "Error loading .json file '" << filename << "': field 'texsize' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' does not exist, is not an array, or is not length 2" << std::endl;
                                        goto dir_loop_end;
                                    }

                                    // get frame offset
                                    if (frame_data.contains("offset") && frame_data["offset"].is_array() && frame_data["offset"].size() == 3) {
                                        // get first value
                                        auto value0 = frame_data["offset"][0];
                                        if (value0.is_number_float()) {
                                            frame.offset.x = value0;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 0 of field 'offset' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }

                                        // get second value
                                        auto value1 = frame_data["offset"][1];
                                        if (value1.is_number_float()) {
                                            frame.offset.y = value1;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 1 of field 'offset' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }

                                        // get third value
                                        auto value2 = frame_data["offset"][2];
                                        if (value2.is_number_float()) {
                                            frame.offset.z = value2;
                                        } else {
                                            std::cerr << "Error loading .json file '" << filename << "': index 2 of field 'offset' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
                                            goto dir_loop_end;
                                        }
                                    } else {
                                        std::cerr << "Error loading .json file '" << filename << "': field 'offset' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' does not exist, is not an array, or is not length 3" << std::endl;
                                        goto dir_loop_end;
                                    }

                                    // get frame duration
                                    if (frame_data.contains("duration") && frame_data["duration"].is_number_integer()) {
                                        frame.duration = frame_data["duration"];
                                    } else {
                                        std::cerr << "Error loading .json file '" << filename << "': field 'duration' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' does not exist or is not an integer" << std::endl;
                                        goto dir_loop_end;
                                    }

                                    cycles.back().addFrame(frame);

                                } else {
                                    std::cerr << "Error loading .json file '" << filename << "': frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not an object" << std::endl;
                                    goto dir_loop_end;
                                }
                            }

                        } else {
                            std::cerr << "Error loading .json file '" << filename << "': field 'frames' of frame of cycle '" << iter_cycles.key() << "' does not exist or is not an object" << std::endl;
                            goto dir_loop_end;
                        }

                        // retrieve loop flag
                        if (data["cycles"][iter_cycles.key()].contains("loop") && data["cycles"][iter_cycles.key()]["loop"].is_boolean()) {
                            cycles.back().setLoop(data["cycles"][iter_cycles.key()]["loop"]);
                        } else {
                            std::cerr << "Error loading .json file '" << filename << "': 'loop' field of cycle '" << iter_cycles.key() << "' is missing or is not an boolean" << std::endl;
                            goto dir_loop_end;
                        }

                    } else {
                        std::cerr << "Error loading .json file '" << filename << "': cycle '" << iter_cycles.key() << "' is not an object" << std::endl;
                        goto dir_loop_end;
                    }
                }
            }
            else {
                std::cerr << "Error loading .json file '" << filename << "': field 'cycles' is missing or is not an object" << std::endl;
                goto dir_loop_end;
            }

            // push loaded animation data to map
            animations[name] = Animation();
            for (unsigned i = 0; i < cycles.size(); i++) {
                animations[name].addCycle(cycles[i]);
            }
        }
        dir_loop_end:
        continue;
    }

    return animations;
}