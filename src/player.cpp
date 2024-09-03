#include "player.hpp"

void Player::_initCharacter() {
    getBox()->dim = glm::vec3(6.0f, 13.0f, 0.0f);
    getAnimState().setAnimState(0);
}

void Player::_baseCharacter() {
    if (getAnimState().completed())
        getAnimState().setAnimState(1);

    playerMotion();
    playerAction();
}

void Player::_killCharacter() {
    int id = getManager()->spawnEntity("PlayerSmoke");
    if (id >= 0) {
        Entity *effect = getManager()->getEntity(id);
        effect->getQuad()->pos.v = getBox()->pos;
    }
}

void Player::_collisionCharacter(Box *box) {
    kill();
}

Player::Player() : Character(glm::vec3(16.0f, 16.0f, 0.0f)), _input(nullptr), _input_ready(false), _accel(0.2f), _deccel(0.15f), _spd_max(0.8f), _cooldown(0.0f), _max_cooldown(15.0f), _prevmovedir(0.0f) {}

void Player::playerSetup(Input *input) {
    _input = input;
    _input_ready = true;
};

void Player::playerMotion() {
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
    
    if (glm::length(dir) > 0)
        _prevmovedir = dir;
}

void Player::playerAction() {
    // get mouse position
    glm::vec2 mousepos = _input->mousepos();
    glm::vec3 dirvec;

    if (glm::abs(mousepos.x) <= 128 && glm::abs(mousepos.y) <= 128) {
        glm::vec3 mousepos3d = glm::vec3(mousepos.x, mousepos.y, 0.0f);

        // get normalized and scaled direction
        dirvec = mousepos3d - getBox()->pos;
        dirvec /= glm::length(dirvec);
    } else
        dirvec = glm::vec3(_prevmovedir.x, _prevmovedir.y, 0.0f);
    
    // spawn projectile if not on cooldown
    if (_cooldown <= 0.0f) {
        if (_input->get_m1() || _input->get_space()) {
            // spawn projectile, set its position, and set cooldown
            int id = getManager()->spawnObject("OrbShot");
            if (id >= 0) {
                Object *shot = getManager()->getObject(id);
                shot->getBox()->pos = getBox()->pos;
                shot->getBox()->vel = dirvec;
            }

            _cooldown = _max_cooldown;
        }
    } else
        _cooldown -= 1.0f;  
}