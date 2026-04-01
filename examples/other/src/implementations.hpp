#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalResources;

const float diag_factor = glm::sin(glm::radians(45.0f));

/*
    Script Interface SpellInterface 
    Describes behavior of a spell. Enqueues self while executing. Calls _startSpell() if new execution loop started. Can call endExec()
    to stop automatic enqueuing.
*/
class SpellInterface : public ScriptInterface {
    unsigned _execution_queue;
    GlobalResources* _resources;

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
class ES_Player : public EntityScriptInterface {
    GLFWInput *_input_state;
    GlobalResources* _resources;
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
    ES_Player(GLFWInput* input_state, GlobalResources* resources);
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

struct GlobalResources {
    GlobalResources(EntityManager* entitymanager);
    
    EntityManager *manager;
    GenericEntityScriptProvider<ES_Chaser> provider_ES_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_ES_Lifetime;
};

// --------------------------------------------------------------------------------------------------------------------------

// ES_Player allocator that holds reference to input state
class PlayerProvider : public EntityScriptProviderInterface<ES_Player> {
    GLFWInput* _input_state;
    GlobalResources* _resources;
    ES_Player* _providerAllocate() override { return new ES_Player(_input_state, _resources); }
public:
    PlayerProvider(GLFWInput* input_state, GlobalResources* resources) : _input_state(input_state), _resources(resources) {}
};

#endif