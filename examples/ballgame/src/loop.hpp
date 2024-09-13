#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "coreinit.hpp"
#include "allocators.hpp"
#include "../../../core/include/text.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <thread>

struct Allocators {
    GenericEntityAllocator<SmallSmoke> SmallSmoke_allocator;
    GenericEntityAllocator<MediumSmoke> MediumSmoke_allocator;
    GenericEntityAllocator<BigSmoke> BigSmoke_allocator;
    GenericEntityAllocator<VeryBigSmoke> VeryBigSmoke_allocator;
    GenericEntityAllocator<PlayerSmoke> PlayerSmoke_allocator;
    GenericEntityAllocator<OrbShotParticle> OrbShotParticle_allocator;
    GenericEntityAllocator<OrbShotBoom> OrbShotBoom_allocator;
    OrbShotAllocator OrbShot_allocator;
    PlayerAllocator Player_allocator;
    SmallBallAllocator SmallBall_allocator;
    MediumBallAllocator MediumBall_allocator;
    BigBallAllocator BigBall_allocator;
    VeryBigBallAllocator VeryBigBall_allocator;
    RingAllocator Ring_allocator;

    // need this to initialize some members
    Allocators(GLFWInput *input, bool *killflag);
};

struct GlobalState {
    int game_state;
    int i;

    float number;
    float rate;
    float target_number;
    float max_rate;
    float rate_increase;
    float rate_decrease;
    int spawn_rate;
    int round;

    bool enter_check;
    bool enter_state;

    bool killflag;

    Text toptext;
    Text subtext;
    Text bottomtext;

    // need this to initialize Text members
    GlobalState(GLEnv *glenv);
};

/* Primary program execution loop. */
void loop(CoreResources *core);

/* Adds allocators to core Executor. */
void addAllocators(CoreResources *core, GlobalState *globalstate, Allocators *allocators);

/* Handles initialization of game state. */
void gameInitialize(CoreResources *core, GlobalState *globalstate);

/* Handles one iteration of game state. */
void gameStep(CoreResources *core, GlobalState *globalstate);

/* Handles processing of core data. */
void gameProcess(CoreResources *core, GlobalState *globalstate);

#endif