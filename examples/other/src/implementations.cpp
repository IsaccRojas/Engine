#include "implementations.hpp"

void ES_Player::_initEntity() {
    entity().attributes3f()["pos"] = glm::vec3(0.0f);
}

void ES_Player::_execEntity() {
    glm::vec3 &pos = entity().attributes3f()["pos"];

    float speed = 0.5f;
    glm::vec3 dir = glm::vec3(
        float(-1.0f * _input_state->get_a()) + float(_input_state->get_d()),
        float(-1.0f * _input_state->get_s()) + float(_input_state->get_w()),
        0.0f
    );
    if (glm::length(dir))
        pos += speed * glm::normalize(dir);

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

// --------------------------------------------------------------------------------------------------------------------------

void ES_Chaser::_initEntity() {
    entity().attributes3f()["pos"] = glm::vec3(0.0f);
}

void ES_Chaser::_execEntity() {
    glm::vec3 &pos = entity().attributes3f()["pos"];

    auto group_player_iter = entity().manager().groupBegin("Group_Player");
    if (group_player_iter != entity().manager().groupEnd("Group_Player")) {
        glm::vec3 player_pos = (*group_player_iter)->attributes3f()["pos"];
    }
    
    enqueueExec(0);
}

void ES_Chaser::_killEntity() {}

void ES_Chaser::_updateEntity() {
    glm::vec3 &pos = entity().attributes3f()["pos"];
    entity().quads()[0]->bv_pos.v = pos;
    entity().boxes()[0]->transform.pos = pos;
}

void ES_Chaser::_receive(Entity *entity, std::string message) {}

ES_Chaser::ES_Chaser() : EntityScript() {}