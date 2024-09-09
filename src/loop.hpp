#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "player.hpp"
#include "effect.hpp"
#include "chaser.hpp"
#include "../core/include/text.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#include <thread>

/* Primary program execution loop.
*/
void loop(GLFWwindow *winhandle);

#endif