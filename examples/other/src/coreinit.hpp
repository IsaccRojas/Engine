#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "C:\dev\include\GL\glew.h"
#include "..\..\..\core\include\glfwstate.hpp"
#include "..\..\..\core\include\glfwinput.hpp"
#include "..\..\..\core\include\entity.hpp"

struct CoreResources {
    GLFWState state;
    GLFWInput input;

    EntityExecutor entityexecutor;
    GLEnv glenv;
    CollisionSpace collisionspace;

    EntityManager entitymanager;
};

/* Initializes core library data structures.
*/
void initializeCore(CoreResources *core);

#endif