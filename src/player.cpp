#include "player.hpp"

void Player::_initCharacter() {
    getBox()->setCorrection(true);
    getBox()->dim = glm::vec3(6.0f, 13.0f, 0.0f);
    getAnimState().setAnimState(0);
}

void Player::_baseCharacter() {
    glm::vec3 &vel = getBox()->vel;
    glm::vec3 vel_i = vel;
    float spd_i = glm::length(vel);
    float dec_factor;

    //get decceleration based on current speed and apply
    if (spd_i != 0.0f)
        dec_factor = _deccel / glm::length(vel);
    else
        dec_factor = 0.0f;
    vel -= vel * dec_factor;

    //determine if deccelerated completely ("passed" 0)
    if ((vel_i.x > 0 && vel.x < 0) || (vel_i.x < 0 && vel.x > 0))
        vel.x = 0.0f;
    if ((vel_i.y > 0 && vel.y < 0) || (vel_i.y < 0 && vel.y > 0))
        vel.y = 0.0f;

    //accelerate based on input
    glm::vec2 dir = _input->inputdir();
    vel += glm::vec3(dir.x * _accel, dir.y * _accel, 0.0f);

    //reduce velocity to max if speed exceeds max
    if (glm::length(vel) > _spd_max)
        vel = glm::normalize(vel) * _spd_max;

    // change animation state based on input
    bool s = _input->get_s(), a = _input->get_a(), d = _input->get_d(), w = _input->get_w();
    if (s)
        _cur_animstate = 0;
    if (a)
        _cur_animstate = 1;
    if (d)
        _cur_animstate = 2;
    if (w)
        _cur_animstate = 3;
    if (!s && !a && !d && !w)
        _cur_animstate = -1;
    checkAnimState();
}

void Player::_killCharacter() {}

void Player::_collisionCharacter(Box *box) {}

Player::Player() : Character(glm::vec3(16.0f, 16.0f, 0.0f)), _input(nullptr), _input_ready(false), _accel(0.2f), _deccel(0.15f), _spd_max(0.8f), _cur_animstate(0), _prev_animstate(0) {}

void Player::playerSetup(Input *input) {
    _input = input;
    _input_ready = true;
};

void Player::checkAnimState() {
    // do nothing if no change
    if (_cur_animstate == _prev_animstate)
        return;
    
    if (_cur_animstate == 0)
        getAnimState().setAnimState(4);
    else if (_cur_animstate == 1)
        getAnimState().setAnimState(5);
    else if (_cur_animstate == 2)
        getAnimState().setAnimState(6);
    else if (_cur_animstate == 3)
        getAnimState().setAnimState(7);
    
    if (_cur_animstate == -1) {
        if (_prev_animstate == 0)
            getAnimState().setAnimState(0);
        if (_prev_animstate == 1)
            getAnimState().setAnimState(1);
        if (_prev_animstate == 2)
            getAnimState().setAnimState(2);
        if (_prev_animstate == 3)
            getAnimState().setAnimState(3);
    }

    _prev_animstate = _cur_animstate;
}