#include "implementations.hpp"

void ES_Player::_initEntity() {
    entity().attributes3f()["pos"] = glm::vec3(0.0f);
}

void ES_Player::_execEntity() {
    glm::vec3 &pos = entity().attributes3f()["pos"];

    float speed = 0.25f;
    if (_input_state->get_w())
        pos += glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_a())
        pos -= glm::vec3(speed, 0.0f, 0.0f);
    if (_input_state->get_s())
        pos -= glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_d())
        pos += glm::vec3(speed, 0.0f, 0.0f);

    if (_input_state->get_space())
        enqueueKill();
    enqueueExec(0);
}

void ES_Player::_killEntity() {}

void ES_Player::_updateEntity() {
    glm::vec3 &pos = entity().attributes3f()["pos"];
    entity().quads()[0]->bv_pos.v = pos;
    entity().boxes()[0]->transform.pos = pos;
}

void ES_Player::_receive(Entity *entity, std::string message) {}

ES_Player::ES_Player(GLFWInput *input_state) : EntityScript(), _input_state(input_state) {}