#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

const float diag_factor = glm::sin(glm::radians(45.0f));

class ES_Player : public EntityScript {
    GLFWInput *_input_state;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *entity, std::string message) override;
    void _collide(Entity *entity) override;
public:
    ES_Player(GLFWInput *input_state);
};

class ES_Chaser : public EntityScript {
    Entity *_target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *entity, std::string message) override;
    void _collide(Entity *entity) override;
public:
    ES_Chaser();
};

#endif