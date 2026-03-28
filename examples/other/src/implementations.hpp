#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalProviders;

const float diag_factor = glm::sin(glm::radians(45.0f));

class ES_Player : public EntityScriptInterface {
    GLFWInput *_input_state;
    GlobalProviders* _providers;
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _hitbox_cooldown_max;
    float _hitbox_cooldown;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* entity, std::string message) override;
    void _collide(Entity* entity) override;
public:
    ES_Player(GLFWInput* input_state, GlobalProviders* providers);
};

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

class ES_Hitbox : public EntityScriptInterface {
    bool _debug;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Hitbox();
    unsigned lifetime;
};

struct GlobalProviders {
    GenericEntityScriptProvider<ES_Chaser> provider_ES_Chaser;
    GenericEntityScriptProvider<ES_Hitbox> provider_ES_Hitbox;
};

#endif