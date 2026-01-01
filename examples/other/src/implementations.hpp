#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwinput.hpp"

class ES_Player : public EntityScript {
    Transform _transform;
    GLFWInput *_input_state;
    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;
public:
    ES_Player(GLFWInput *input_state);
};

#endif