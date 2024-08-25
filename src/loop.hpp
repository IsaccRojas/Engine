#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "effect.hpp"
#include "player.hpp"
#include "orb.hpp"
#include "slime.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#include <thread>

/* Primary program execution loop.
*/
void loop(GLFWwindow *winhandle);

#endif