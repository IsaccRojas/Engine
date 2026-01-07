#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

class ES_Player : public EntityScript {
    GLFWInput *_input_state;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;

    void _receive(Entity *entity, std::string message) override;
public:
    ES_Player(GLFWInput *input_state);
};

#endif