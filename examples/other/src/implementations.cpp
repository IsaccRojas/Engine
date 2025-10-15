#include "implementations.hpp"

void Player::_initObject() {
    transform.pos = glm::vec3(0.0f);
    transform.scale = glm::vec3(16.0f, 16.0f, 0.0f);
}
void Player::_baseObject() {
    float speed = 0.25f;
    if (_input_state->get_w())
        transform.pos += glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_a())
        transform.pos -= glm::vec3(speed, 0.0f, 0.0f);
    if (_input_state->get_s())
        transform.pos -= glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_d())
        transform.pos += glm::vec3(speed, 0.0f, 0.0f);
}
void Player::_killObject() {}
void Player::_onCollision(Box *other) {}

Player::Player(std::string animation_name, std::string filter_name, GLFWInput *input_state) : 
    Object(animation_name, filter_name),
    _input_state(input_state)
{}

// ======================================================================================================

void ProjectileBasic::_initObject() {
    transform.pos = glm::vec3(0.0f);
    transform.scale = glm::vec3(16.0f, 16.0f, 0.0f);
}
void ProjectileBasic::_baseObject() {}
void ProjectileBasic::_killObject() {}
void ProjectileBasic::_onCollision(Box *other) {}

ProjectileBasic::ProjectileBasic(std::string animation_name, std::string filter_name) : 
    Object(animation_name, filter_name)
{}