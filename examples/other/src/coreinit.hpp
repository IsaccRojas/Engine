#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "C:\dev\include\GL\glew.h"
#include "..\..\..\core\include\glfwstate.hpp"
#include "..\..\..\core\include\glfwinput.hpp"
#include "..\..\..\core\include\entity.hpp"

#include "implementations.hpp"

struct CoreResources {
    CoreResources();

    GLFWState state;
    GLFWInput input;

    std::unordered_map<std::string, Animation> animations;
    std::unordered_map<std::string, Filter> filters;

    EntityScriptExecutor executor;
    GLEnv glenv;
    CollisionSpace collisionspace;
    EntityManager entitymanager;

    PlayerProvider provider_ES_Player;
    GlobalResources globalresources;
};

/* Initializes core library data structures.
*/
void initializeCore(CoreResources *core);

#endif