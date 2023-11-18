#include "animation.hpp"

Cycle::Cycle(bool loop) : _loop(loop) {}
Cycle::Cycle(const Cycle &other) : _loop(other._loop), _frames(other._frames) {}
Cycle::~Cycle() {}

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
Animation::Animation(const Animation &other) : _cycles(other._cycles) {}
Animation::~Animation() {}

/* Adds frame to cycle; added to the end of the cycle, so make sure to call this on frames
corresponding to the desired order of the frames.
*/
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

AnimationState::AnimationState(Animation *animation) :
    _step(0), 
    _framestate(0), 
    _cyclestate(0), 
    _completed(false), 
    _animation(animation)
{
    _currentcycle = &(_animation->getCycle(_cyclestate));
    _currentframe = &(_currentcycle->getFrame(_framestate));
}
AnimationState::AnimationState() :
    _step(0),
    _framestate(0),
    _cyclestate(0),
    _completed(false),
    _animation(nullptr),
    _currentcycle(nullptr),
    _currentframe(nullptr)
{}
AnimationState::AnimationState(const AnimationState &other) : 
    _step(other._step), 
    _framestate(other._framestate), 
    _cyclestate(other._cyclestate), 
    _completed(other._completed), 
    _animation(other._animation),
    _currentcycle(other._currentcycle),
    _currentframe(other._currentframe)
{}
AnimationState::~AnimationState() {}

void AnimationState::setAnimation(Animation *animation) {
    _step = 0;
    _framestate = 0;
    _cyclestate = 0;
    _completed = false;
    _animation = animation;
    _currentcycle = &(_animation->getCycle(_cyclestate));
    _currentframe = &(_currentcycle->getFrame(_framestate));
}

/* Sets the animation state, using the cycle corresponding to the provided state 
    for future operations.
*/
void AnimationState::setAnimState(int state) {
    if (state == _cyclestate)
        return;
    _cyclestate = state;
    _currentcycle = &(_animation->getCycle(_cyclestate));
    this->setCycleState(0);
}

/* Sets the cycle state, using the frame corresponding to the provided state 
    for future operations.
*/
void AnimationState::setCycleState(int state) {
    _framestate = state;
    _currentframe = &(_currentcycle->getFrame(_framestate));
    _step = 0;
    _completed = false;
}

/* Advances the cycle one step; will go to the next frame if the current frame's duration is
    exceeded; will loop or stop if last frame's duration is exceeded.
*/
void AnimationState::step() {
    // check if completed or if no frames exist
    if (_completed || _currentcycle->count() == 0)
        return;
    
    // advance one step in time
    _step++;

    // check if duration of current framestate is over
    if (_step >= _currentframe->duration) {
        // check if on last framestate
        if (_framestate + 1 >= _currentcycle->count()) {
            // check if we should loop
            if (_currentcycle->loops()) {
                this->setCycleState(0);
                return;
            }

            // otherwise, stay here
            _completed = true;
            return;
        }

        // otherwise, go to next frame and reset time
        this->setCycleState(_framestate + 1);
    }
}

/* Gets the current frame of the cycle. Causes an error if no frames exist.
*/
Frame *AnimationState::getCurrent() {
    return _currentframe;
}

/* Returns whether the cycle has completed or not (always false if looping is set to true).
*/
bool AnimationState::completed() {
    return _completed;
}

/* Checks if provided string ends with the provided suffix.
*/
static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

/* Checks if provided string starts with the provided prefix
*/
static bool startsWith(const std::string& str, const std::string& prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

/* Searches the provided directory for .json files, and parses them to load animation data. Returns
   an unordered map mapping .json file names (excluding the .json extension) to their defined
   Animation data.

   All .json files parsed are expected to have the following format:

   e.g.
   {
        "name" : "example",
        "cycles" : {
            "cyclename1" : {
                "frames" : {
                    "framename1" : {
                        "texpos" : [0.0, 1.0],
                        "texsize" : [2.0, 3.0],
                        "offset" : [4.0, 5.0, 6.0],
                        "duration" : 4
                    },
                    "framename2" : {
                        "texpos" : [7.0, 8.0],
                        "texsize" : [9.0, 10.0],
                        "offset" : [11.0, 12.0, 13.0],
                        "duration" : 8
                    }
                    // ...
                },
                loop : true
            },
            "cyclename2" : {
                "frames" : {
                    "framename3" : {
                        // ...
                    }
                    // ...
                },
                loop : false
            }
            // ...
        }
   }

   An arbitrary number of objects corresponding to cycles can be defined in the "cycles" field. An
   arbitrary number of objects corresponding to frames can be defined in the "frames" field of a "cycle"
   object.
*/
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
                                            std::cerr << "Error loading .json file '" << filename << "': index 1 of field 'texpos' of frame '" << iter_frames.key() << "' of cycle '" << iter_cycles.key() << "' is not a floating point number" << std::endl;
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
            for (int i = 0; i < cycles.size(); i++) {
                animations[name].addCycle(cycles[i]);
            }
        }
        dir_loop_end:
        0;
    }

    return animations;
}