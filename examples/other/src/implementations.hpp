#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

const float diag_factor = glm::sin(glm::radians(45.0f));

struct GlobalResources;

/* class ResourcesMixin
   Mix-in class for accessing GlobalResources reference.
*/
class ResourcesMixin {
    GlobalResources *_resources;

public:
    ResourcesMixin(GlobalResources* resources);
    GlobalResources& resources();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports ResourcesMixin.
*/
template<typename T>
class ScriptResourcesProvider : public ScriptProviderInterface<T> {
    GlobalResources* _resources;
    T* _providerAllocate() override { return new T(_resources); }
public:
    ScriptResourcesProvider(GlobalResources* resources) : _resources(resources) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityScriptResourcesProvider
   Templated implementation of EntityScriptProviderInterface that supports ResourcesMixin.
*/
template<typename T>
class EntityScriptResourcesProvider : public EntityScriptProviderInterface<T> {
    GlobalResources* _resources;
    T* _providerAllocate() override { return new T(_resources); }
public:
    EntityScriptResourcesProvider(GlobalResources* resources) : _resources(resources) {}
};

// --------------------------------------------------------------------------------------------------------------------------

class S_Spell_LightBall : public ScriptInterface, public ResourcesMixin {
    void _init() override;
    void _exec() override;
    void _kill() override;
    void _update() override;
public:
    S_Spell_LightBall(GlobalResources* resources);
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Player 
    Player script.
*/
class ES_Player : public EntityScriptInterface, public ResourcesMixin {
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _hitbox_cooldown_max;
    float _hitbox_cooldown;
    float _speed;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
public:
    ES_Player(GlobalResources* resources);
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Chaser
    Chases assigned target directly.
*/
class ES_Chaser : public EntityScriptInterface, public ResourcesMixin {
    Entity *_target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Chaser(GlobalResources* resources);
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class ES_Lifetime : public EntityScriptInterface {
    Entity* _target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Lifetime();
    unsigned lifetime;
    glm::vec3 vel;
};

// --------------------------------------------------------------------------------------------------------------------------

/* struct Global Resources
   Aggregates resources for classes with ResourcesMixin inherited to access.
*/
struct GlobalResources {
    GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput);

    EntityManager* manager;
    EntityScriptExecutor* executor;
    GLFWInput* input;
    
    EntityScriptResourcesProvider<ES_Player> provider_Player;
    EntityScriptResourcesProvider<ES_Chaser> provider_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_Lifetime;
    ScriptResourcesProvider<S_Spell_LightBall> provider_Spell_LightBall;
};

#endif