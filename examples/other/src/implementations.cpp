#include "implementations.hpp"

void ES_Player::_initEntity() {
    _transform.pos = glm::vec3(0.0f);
    _transform.scale = glm::vec3(16.0f);
}
void ES_Player::_baseEntity() {
    float speed = 0.25f;
    if (_input_state->get_w())
        _transform.pos += glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_a())
        _transform.pos -= glm::vec3(speed, 0.0f, 0.0f);
    if (_input_state->get_s())
        _transform.pos -= glm::vec3(0.0f, speed, 0.0f);
    if (_input_state->get_d())
        _transform.pos += glm::vec3(speed, 0.0f, 0.0f);

    Quad *quad = entity().quads()[0];
    quad->bv_pos.v = _transform.pos;
    quad->bv_scale.v = _transform.scale;

    entity().boxes()[0]->transform = _transform;

    if (_input_state->get_space())
        entity().manager().removeEntity(&entity());
    
    enqueueExec(0);
}
void ES_Player::_killEntity() {}

ES_Player::ES_Player(GLFWInput *input_state) : EntityScript(), _input_state(input_state) {}