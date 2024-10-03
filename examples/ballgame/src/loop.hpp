#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "coreinit.hpp"
#include "implementations.hpp"
#include "../../../core/include/text.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <thread>

// receives enemies and players to initialize them
struct GlobalState : public Receiver<Enemy>, public Receiver<Player> {
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

    GLFWInput *input;
    bool killflag;

    Text toptext;
    Text subtext;
    Text bottomtext;

    // need this to initialize Text members
    GlobalState(GLEnv *glenv, GLFWInput *glfwinput);

    std::queue<int> size_factors;

    void _receive(Enemy *enemy) override;
    void _receive(Player *player) override;
};

/* Primary program execution loop. */
void loop(CoreResources *core);

/* Handles initialization of game state. */
void gameInitialize(CoreResources *core, GlobalState *globalstate);

/* Handles one iteration of game state. */
void gameStep(CoreResources *core, GlobalState *globalstate, Providers *providers);

/* Handles processing of core data. */
void gameProcess(CoreResources *core, GlobalState *globalstate);

#endif