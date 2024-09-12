#ifndef LOOP_HPP_
#define LOOP_HPP_

#include "coreinit.hpp"
#include "implementations.hpp"
#include "../core/include/text.hpp"

#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <thread>

/* Primary program execution loop.
*/
void loop(CoreResources *core);

#endif