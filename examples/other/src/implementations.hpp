#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "resource.hpp"
#include "../../../core/include/glfwinput.hpp"

const float diag_factor = glm::sin(glm::radians(45.0f));

struct GlobalResources;

class S_Spell_LightBall : public ScriptInterface, public Resource<GlobalResources> {
    void _init() override;
    void _exec() override;
    void _kill() override;
    void _update() override;
public:
    S_Spell_LightBall();
    glm::vec3 src_pos;
    glm::vec3 vel;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Player 
    Player script.
*/
class ES_Player : public EntityScriptInterface, public Resource<GlobalResources> {
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
    ES_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Chaser
    Chases assigned target directly.
*/
class ES_Chaser : public EntityScriptInterface, public Resource<GlobalResources> {
    Entity *_target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Chaser();
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
   Aggregates resources for classes with Resource<GlobalResources> inherited to access.
*/
struct GlobalResources {
    GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput);

    EntityManager* manager;
    EntityScriptExecutor* executor;
    GLFWInput* input;
    
    EntityScriptResourceProvider<ES_Player, GlobalResources> provider_Player;
    EntityScriptResourceProvider<ES_Chaser, GlobalResources> provider_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_Lifetime;
    ScriptResourceProvider<S_Spell_LightBall, GlobalResources> provider_Spell_LightBall;
};

#endif