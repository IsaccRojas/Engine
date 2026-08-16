#ifndef COREINIT_HPP_
#define COREINIT_HPP_

#include "assets.hpp"
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

    GlobalState globalstate;

    GSEntityScriptAllocator<ES_Correction> allocator_Correction;
    GSEntityScriptAllocator<ES_Player> allocator_Player;
    GSEntityScriptAllocator<ES_Mover> allocator_Mover;
    GSEntityScriptAllocator<ES_Pickup> allocator_Pickup;
    GSEntityScriptAllocator<ES_Stairs> allocator_Stairs;
    GSEntityScriptAllocator<ES_BreakableTile> allocator_BreakableTile;
    GenericProvidingEntityScriptAllocator<ES_Lifetime> allocator_Lifetime;
    GSScriptAllocator<Spell_LightBallSpell> allocator_Spell_LightBallSpell;
};

/* Initializes core library data structures. */
void initializeCore(CoreResources *core);

/* Initializes script, graphics, and collision assets. */
void initializeAssets(CoreResources *core);

#endif