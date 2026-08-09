#ifndef ANIMATION_HPP_
#define ANIMATION_HPP_

#include <iostream>
#include <vector>
#include <unordered_map>
#include "C:\dev\include\glm\glm.hpp"
#include <filesystem>
#include <fstream>
#include "json.hpp"

/* struct Frame
   Represents a graphical frame; the data corresponding to a single "image" in an animation.
   - texpos - texture position
   - texsize - width and height of texture
   - duration - number of time steps this frame lasts
*/
struct Frame {
    glm::vec3 texpos;
    glm::vec2 texsize;
    unsigned duration;
    Frame(glm::vec3 textureposition, glm::vec2 texturesize, unsigned frameduration);
    Frame();
};

/* class Cycle
   Represents a set of frames that form a cycle of images.
*/
class Cycle {
    std::string _name;
    bool _loop;
    std::vector<Frame> _frames;
public:
    Cycle(const char* name, bool loop, std::vector<Frame> frames);
    Cycle();
    ~Cycle();

    // default copy assignment/construction are fine

    /* Sets name of cycle. */
    Cycle& setName(const char* name);

    /* Sets whether this cycle loops or not. */
    Cycle& setLoop(bool loop);

    /* Adds frame to end of cycle. */
    Cycle& addFrame(const Frame& frame);

    std::string name();

    bool loop();

    Frame& frame(unsigned i);

    /* Returns number of frames contained in this cycle. */
    unsigned count() const;
};

/* class Animation
   Represents a set of cycles that form an animation.
*/
class Animation {
    std::string _name;
    std::vector<Cycle> _cycles;
    
    std::unordered_map<std::string, unsigned> _cycle_indices;
public:
    Animation(const char* name, std::vector<Cycle> cycles);
    Animation();
    ~Animation();

    // default copy assignment/construction are fine

    /* Sets name of animation. */
    Animation& setName(const char* name);

    /* Adds cycle to end of animation. */
    Animation& addCycle(const Cycle& cycle);

    std::string name();

    Cycle& cycle(unsigned i);
    Cycle& cycle(const char* name);

    unsigned cycleIndex(const char* name);

    /* Returns number of cycles contained in this animation. */
    unsigned count();
};

/* class AnimationState
   Provides a view of an animation, allowing the client to step through a provided animation's
   frames in order, based on the internal frame durations. Holds state information to control
   this stepping.
*/
class AnimationState {
    Animation* _animation;
    Cycle* _current_cycle;
    Frame* _current_frame;
    unsigned _step;

    // variables for indexing cycle and animation, respectively
    unsigned _cycle_state;
    unsigned _frame_state;
    bool _completed;

public:
    AnimationState(Animation* animation);
    AnimationState();
    ~AnimationState();

    // default copy assignment/construction are fine (references are read only)

    /* Sets up instance to preserve state of provided animation. */
    void setAnimation(Animation* animation);

    /* Sets the animation cycle, using the cycle corresponding to the provided integer or 
       for name future operations. Does nothing if the cycle provided is the same as the 
       current one.
    */
    void setCycleState(unsigned i);
    void setCycleState(const char* name);

    /* Sets the animation frame, using the frame corresponding to the provided integer for
       future operations.
    */
    void setFrameState(unsigned i);

    /* Advances the cycle one step; will go to the next frame if the current frame's duration is
       exceeded; will loop or stop if last frame's duration is exceeded.
    */
    void step();

    /* Gets the current frame of the cycle. Causes an error if no frames exist. */
    const Frame& current();

    /* Returns whether or not this AnimationState is set to a specific Animation. */
    bool hasAnimation();

    /* Returns whether or not this AnimationState's contained Animation is empty. */
    bool animationEmpty();

    /* Returns whether the cycle has completed or not (always false if looping is set to true). */
    bool completed();
};

#endif