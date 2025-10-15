#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "coreinit.hpp"
#include "../../../core/include/text.hpp"

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <chrono>
#include <thread>

#include "implementations.hpp"

/* Primary program execution loop. */
void loop(CoreResources *core);

#endif