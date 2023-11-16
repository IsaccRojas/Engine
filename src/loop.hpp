#pragma once

#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "object.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>

const float PI = 3.14159265f;

/* Primary program execution loop. Uses an instance of GLEnv to handle graphics.
*/
void loop(GLFWwindow *winhandle);

#endif