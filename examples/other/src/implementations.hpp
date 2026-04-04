#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

const float diag_factor = glm::sin(glm::radians(45.0f));

class ES_Chaser;
class ES_Lifetime;

/* struct Global Resources
   Aggregates resources for classes with ResourcesMixin inherited to access.
*/
struct GlobalResources {
    GlobalResources(GLFWInput* glfw_input);
    GLFWInput* input;
    GenericEntityScriptProvider<ES_Chaser> provider_ES_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_ES_Lifetime;
};

// --------------------------------------------------------------------------------------------------------------------------

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

/* class ResourcesMixin
   Templated implementation of EntityScriptProviderInterface that supports ResourcesMixin.
*/
template<typename T>
class ResourcesProvider : public EntityScriptProviderInterface<T> {
    GlobalResources* _resources;
    T* _providerAllocate() override { return new T(_resources); }
public:
    ResourcesProvider(GlobalResources* resources) : _resources(resources) {}
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Script Interface SpellInterface 
    Describes behavior of a spell. Enqueues self while executing. Calls _startSpell() if new execution loop started. Can call endExec()
    to stop automatic enqueuing.
*/
class SpellInterface : public ScriptInterface, public ResourcesMixin {
    unsigned _execution_queue;

    // controls when to call _startSpell()
    bool _executing;

    // controls when to stop enqueuing and reset _executing
    bool _end_execution;

    virtual void _init() override;
    virtual void _exec() override;
    virtual void _kill() override;
    virtual void _update() override;

    virtual void _startSpell() = 0;
    virtual void _execSpell() = 0;

public:
    SpellInterface(unsigned execution_queue, GlobalResources* resources);

    void endSpell();
};

// --------------------------------------------------------------------------------------------------------------------------

class Spell_LightBall : public SpellInterface {
    void _startSpell();
    void _execSpell();
public:
    Spell_LightBall(unsigned execution_queue, GlobalResources* resources);
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
class ES_Chaser : public EntityScriptInterface {
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

#endif