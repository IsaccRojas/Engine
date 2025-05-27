#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/text.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct CoreResources {
    GLFWState state;
    GLFWInput input;
    GLEnv glenv;
    unordered_map_string_Animation_t animations;
    unordered_map_string_Filter_t filters;
    PhysSpace<Box> box_space;
    PhysSpace<Sphere> sphere_space;
    EntityExecutor executor;
};

/* Initializes core library data structures.
*/
void initializeCore(CoreResources *core);

#endif