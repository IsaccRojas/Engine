#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "effect.hpp"
#include "object.hpp"
#include "../../../core/include/glfwinput.hpp"

class Player : public Object {
    void _initObject() override;
    void _baseObject() override;
    void _killObject() override;
    void _onCollision(Box *other) override;

    GLFWInput *_input_state;
public:
    Player(std::string animation_name, std::string filter_name, GLFWInput *input_state);
};

class ProjectileBasic : public Object {
    void _initObject() override;
    void _baseObject() override;
    void _killObject() override;
    void _onCollision(Box *other) override;

    GLFWInput *_input_state;
public:
    ProjectileBasic(std::string animation_name, std::string filter_name);
};

#endif