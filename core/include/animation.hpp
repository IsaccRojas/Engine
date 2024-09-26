#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include <iostream>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <filesystem>
#include <fstream>
#include "json.hpp"
#include "util.hpp"

/* struct Frame
   Represents a graphical frame; the data corresponding to a single "image" in an animation.
   - texpos - texture position
   - texsize - width and height of texture
   - scale - scale to apply to instance using this data
   - duration - number of time steps this frame lasts
*/
struct Frame {
    glm::vec3 texpos;
    glm::vec2 texsize;
    glm::vec3 scale;
    unsigned duration;
};

/* class Cycle
   Represents a set of frames that can be added or removed.
*/
class Cycle {
    std::vector<Frame> _frames;
    bool _loop;
    
public:
    Cycle(bool loop);
    Cycle();
    ~Cycle();

    // default copy assignment/construction are fine

    /* Adds frame to cycle; added to the end of the cycle, so make sure to call this on frames
    corresponding to the desired order of the frames.
    */
    Cycle& addFrame(glm::vec3 texpos, glm::vec2 texsize, glm::vec3 scale, unsigned duration);
    Cycle& addFrame(const Frame &frame);

    /* Sets whether this cycle loops or not. */
    void setLoop(bool loop);

    Frame& frame(unsigned i);

    /* Returns number of frames contained in this cycle. */
    unsigned count() const;

    bool loops() const;
};

/* class Animation
   Represents a set of cycles that can be added or removed, collectively forming an animation.
*/
class Animation {
    std::vector<Cycle> _cycles;
public:
    Animation();
    ~Animation();

    // default copy assignment/construction are fine

    /* Adds frame to cycle; added to the end of the cycle, so make sure to call this on frames
    corresponding to the desired order of the frames.
    */
    Animation& addCycle(Cycle &cycle);

    Cycle& cycle(unsigned i);

    /* Returns number of cycles contained in this animation. */
    unsigned count();
};

/* class AnimationState
   Provides a view of an animation, allowing the client to step through a provided animation's
   frames in order, based on the internal frame durations. Holds state information to control
   this stepping.
*/
class AnimationState {
    Animation *_animation;
    Cycle *_current_cycle;
    Frame *_current_frame;
    unsigned _step;

    // variables for indexing cycle and animation, respectively
    unsigned _frame_state;
    unsigned _cycle_state;
    bool _completed;

public:
    AnimationState(Animation *animation);
    AnimationState();
    ~AnimationState();

    // default copy assignment/construction are fine (references are read only)

    /* Sets up instance to preserve state of provided animation. */
    void setAnimation(Animation *animation);

    /* Sets the animation cycle, using the cycle corresponding to the provided integer for
       future operations. Does nothing if the cycle provided is the same as the current one.
    */
    void setCycleState(unsigned cycle_state);

    /* Sets the animation frame, using the frame corresponding to the provided integer for
       future operations.
    */
    void setFrameState(unsigned frame_state);

    /* Advances the cycle one step; will go to the next frame if the current frame's duration is
       exceeded; will loop or stop if last frame's duration is exceeded.
    */
    void step();

    /* Gets the current frame of the cycle. Causes an error if no frames exist. */
    const Frame &current();

    /* Returns whether this AnimationState is set to a specific Animation. */
    bool hasAnimation();

    /* Returns whether the cycle has completed or not (always false if looping is set to true). */
    bool completed();
};

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
                        "scale" : [4.0, 5.0, 6.0],
                        "duration" : 4
                    },
                    "framename2" : {
                        "texpos" : [7.0, 8.0],
                        "texsize" : [9.0, 10.0],
                        "scale" : [11.0, 12.0, 13.0],
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
std::unordered_map<std::string, Animation> loadAnimations(std::string dir);

#endif