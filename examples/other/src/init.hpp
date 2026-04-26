#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "C:\dev\include\GL\glew.h"
#include "..\..\..\core\include\glfwstate.hpp"
#include "..\..\..\core\include\glfwinput.hpp"
#include "..\..\..\core\include\entity.hpp"

#include "implementations.hpp"

struct CoreResources {
    CoreResources();

    GLFWState glfwstate;
    GLFWInput glfwinput;

    std::unordered_map<std::string, Animation> animations;
    std::unordered_map<std::string, Filter> filters;

    EntityScriptExecutor entityscriptexecutor;
    GLEnv glenv;
    CollisionSpace collisionspace;
    EntityManager entitymanager;

    GlobalResources globalresources;

    EntityScriptResourceProvider<ES_Player, GlobalResources, ES_Player, ObjectInterface> provider_Player;
    EntityScriptResourceProvider<ES_Chaser, GlobalResources, ES_Chaser, ObjectInterface> provider_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_Lifetime;
    ScriptResourceProvider<Spell_LightBallSpell, GlobalResources, SpellInterface> provider_Spell_LightBallSpell;
};

/* Initializes core library data structures.
*/
void initializeCore(CoreResources *core);

/* Initializes script, graphics, and collision assets.
*/
void initializeAssets(CoreResources *core);

#endif