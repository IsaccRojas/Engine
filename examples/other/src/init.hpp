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

    GlobalResources globalresources;

    ResourceEntityScriptAllocator<ES_Correction, GlobalResources> allocator_Correction;
    ResourceEntityScriptAllocator<ES_Player, GlobalResources> allocator_Player;
    ResourceEntityScriptAllocator<ES_Mover, GlobalResources> allocator_Mover;
    ResourceEntityScriptAllocator<ES_Pickup, GlobalResources> allocator_Pickup;
    ResourceEntityScriptAllocator<ES_Stairs, GlobalResources> allocator_Stairs;
    ResourceEntityScriptAllocator<ES_BreakableTile, GlobalResources> allocator_BreakableTile;
    GenericProvidingEntityScriptAllocator<ES_Lifetime> allocator_Lifetime;
    ResourceScriptAllocator<Spell_LightBallSpell, GlobalResources> allocator_Spell_LightBallSpell;
};

/* Initializes core library data structures. */
void initializeCore(CoreResources *core);

/* Initializes script, graphics, and collision assets. */
void initializeAssets(CoreResources *core);

#endif