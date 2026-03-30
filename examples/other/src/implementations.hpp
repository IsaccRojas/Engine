#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalProviders;

const float diag_factor = glm::sin(glm::radians(45.0f));

/*
    Entity Script Player 
    Player script.
*/
class ES_Player : public EntityScriptInterface {
    GLFWInput *_input_state;
    GlobalProviders* _providers;
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
    ES_Player(GLFWInput* input_state, GlobalProviders* providers);
};

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

/*
    Entity Script Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    Supports message "target" - assigns target Entity to follow
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

struct GlobalProviders {
    GenericEntityScriptProvider<ES_Chaser> provider_ES_Chaser;
    GenericEntityScriptProvider<ES_Lifetime> provider_ES_Lifetime;
};

// ES_Player allocator that holds reference to input state
class PlayerProvider : public EntityScriptProviderInterface<ES_Player> {
    GLFWInput* _input_state;
    GlobalProviders* _providers;
    ES_Player* _providerAllocate() override { return new ES_Player(_input_state, _providers); }
public:
    PlayerProvider(GLFWInput* input_state, GlobalProviders* providers) : _input_state(input_state), _providers(providers) {}
};

#endif