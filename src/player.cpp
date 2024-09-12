#include "player.hpp"

void OrbShot::_initBasic() {
    _i = 0;
    _lifetime = 119;
    getBox()->vel = _direction;
    getAnimState().setCycleState(0);
}

void OrbShot::_baseBasic() {
    _i++;

    if (_i % 10 == 0) {
        getExecutor()->enqueueSpawnEntity("OrbShotParticle", 1, -1, getBox()->pos);
    }

    if (_i >= _lifetime || getBox()->getCollided())
        enqueueKill();
}

void OrbShot::_killBasic() {
    getExecutor()->enqueueSpawnEntity("OrbShotBoom", 1, -1, getBox()->pos);
}

void OrbShot::_collisionBasic(Box *box) {}

OrbShot::OrbShot() : Basic(glm::vec3(6.0f, 6.0f, 1.0f), glm::vec3(4.0f, 4.0f, 0.0f)), _i(0), _lifetime(119), _direction(0.0f) {}

void OrbShot::setDirection(glm::vec3 direction) { _direction = direction; }

// --------------------------------------------------------------------------------------------------------------------------

void Player::_initBasic() {
    getAnimState().setCycleState(0);
    setChannel(getExecutorID());
    enableReception(true);
}

void Player::_baseBasic() {
    if (getAnimState().completed())
        getAnimState().setCycleState(1);

    if (getBox()->getCollided())
        enqueueKill();

    playerMotion();
    playerAction();
}

void Player::_killBasic() {
    getExecutor()->enqueueSpawnEntity("PlayerSmoke", 1, -1, getBox()->pos);
    enableReception(false);
}

void Player::_collisionBasic(Box *box) {}

void Player::_receive(OrbShot *orbshot) { orbshot->setDirection(_dirvec); }

Player::Player(Input *input) : 
    Basic(glm::vec3(16.0f, 16.0f, 0.0f), glm::vec3(6.0f, 13.0f, 0.0f)), 
    _input(input),
    _accel(0.2f), 
    _deccel(0.15f), 
    _spd_max(0.8f), 
    _cooldown(0.0f), 
    _max_cooldown(15.0f), 
    _prevmovedir(0.0f),
    _dirvec(0.0f)
{}

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

    if (glm::abs(mousepos.x) <= 128 && glm::abs(mousepos.y) <= 128) {
        glm::vec3 mousepos3d = glm::vec3(mousepos.x, mousepos.y, 0.0f);

        // get normalized and scaled direction
        _dirvec = mousepos3d - getBox()->pos;
        _dirvec /= glm::length(_dirvec);
    } else
        _dirvec = glm::vec3(_prevmovedir.x, _prevmovedir.y, 0.0f);
    
    // spawn projectile if not on cooldown
    if (_cooldown <= 0.0f) {
        if (_input->get_m1() || _input->get_space()) {
            // spawn projectile and set cooldown
            getExecutor()->enqueueSpawnObject("OrbShot", 0, getExecutorID(), getBox()->pos);
            _cooldown = _max_cooldown;
        }
    } else
        _cooldown -= 1.0f;
}