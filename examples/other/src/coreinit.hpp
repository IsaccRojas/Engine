#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "gl/glew.h"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"
#include "../../../core/include/entity.hpp"

struct CoreResources {
    GLFWState state;
    GLFWInput input;

    unordered_map_string_Animation_t animations;
    unordered_map_string_Filter_t filters;

    EntityExecutor entityexecutor;
    GLEnv glenv;
    PhysSpace<Box> physspace_box;

    EntityManager entitymanager;
};

/* Initializes core library data structures.
*/
void initializeCore(CoreResources *core);

#endif