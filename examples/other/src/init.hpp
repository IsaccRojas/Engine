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

    EntityScriptResourceProvider<ES_Correction, GlobalResources, ES_Correction> provider_Correction;
    EntityScriptResourceProvider<ES_Player, GlobalResources, ES_Player> provider_Player;
    EntityScriptResourceProvider<ES_Chaser, GlobalResources, ES_Chaser> provider_Chaser;
    EntityScriptResourceProvider<ES_Pickup, GlobalResources, ES_Pickup> provider_Pickup;
    EntityScriptResourceProvider<ES_Stairs, GlobalResources, ES_Stairs> provider_Stairs;
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