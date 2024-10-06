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

/* Primary program execution loop. */
void loop(CoreResources *core);

/* Handles initialization of game state. */
void gameInitialize(CoreResources *core, GlobalState *globalstate);

/* Handles one iteration of game state. */
void gameStep(CoreResources *core, GlobalState *globalstate);

/* Handles processing of core data. */
void gameProcess(CoreResources *core, GlobalState *globalstate);

#endif