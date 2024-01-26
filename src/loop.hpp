#pragma once

#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "../core/include/object.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>

#include <thread>

const float PI = 3.14159265f;

/* Primary program execution loop.
*/
void loop(GLFWwindow *winhandle);

#endif