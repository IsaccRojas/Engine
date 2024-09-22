#include "implementations.hpp"

void Bullet::_initPhysBall() {
    _i = 0;
    vel = _direction;

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(6.0f, 6.0f, 0.0f)};
}

void Bullet::_basePhysBall() {
    _i++;

    if (_i % 10 == 0) {
        //executor().enqueueSpawnEntity("BulletParticle", 1, -1, transform);
    }

    if (_i >= _lifetime || box()->getCollided())
        enqueueKill();
}

void Bullet::_killPhysBall() {
    removeFromProvider();
}

Bullet::Bullet() : PhysBall("", "Bullet"), _i(0), _lifetime(119), _direction(0.0f) {}

void Bullet::setDirection(glm::vec3 direction) { _direction = direction; }

// --------------------------------------------------------------------------------------------------------------------------

void Player::_initPhysBall() {
    setChannel(getExecutorID());
    enableReception(true);

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    
    // override transform scale
    transform = Transform{transform.pos, glm::vec3(12.0f, 12.0f, 0.0f)};
}

void Player::_basePhysBall() {
    if (box()->getCollided())
        enqueueKill();

    playerMotion();
    playerAction();
}

void Player::_killPhysBall() {
    enableReception(false);
    removeFromProvider();
}

void Player::_receive(Bullet *bullet) { bullet->setDirection(_dirvec); }

Player::Player(GLFWInput *input) : 
    PhysBall("", "Player"),
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
        _dirvec = mousepos3d - box()->transform.pos;
        _dirvec /= glm::length(_dirvec);
    } else
        _dirvec = glm::vec3(_prevmovedir.x, _prevmovedir.y, 0.0f);
    
    // spawn projectile if not on cooldown
    if (_cooldown <= 0.0f) {
        if (_input->get_m1() || _input->get_space()) {
            // spawn projectile and set cooldown
            executor().enqueueSpawnEntity("Bullet", 0, getExecutorID(), transform);
            _cooldown = _max_cooldown;
        }
    } else
        _cooldown -= 1.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Enemy::_initPhysBall() {
    // display on top of other entities
    quad()->bv_pos.v.z = 1.0f;
    quad()->bv_color.v = glm::vec4(0.2116f, 0.2116f, 0.2166f, 1.0f);
}

void Enemy::_basePhysBall() {
    if (*_killflag) {
        enqueueKill();
        return;
    }
    
    if (box()->getCollided())
        enemyCollision();
    enemyMotion();

    _t++;
}

void Enemy::_killPhysBall() {
    enableReception(false);
}

Entity *Enemy::_getTarget() {
    // return first ID found
    auto players = getAllProvided();
    if (players)
        for (auto &player : *players)
            return player;
    
    return nullptr;
}

Enemy::Enemy(bool *killflag) :
    PhysBall("", "Enemy"),
    _accel(0.075f), 
    _deccel(0.05f), 
    _spd_max(0.15f), 
    _t(rand() % 256), 
    _prevdir(0.0f),
    _health(1.0f),
    _killflag(killflag)
{}

void Enemy::enemyMotion() {
    // get a target
    Entity *target = _getTarget();

    glm::vec3 vel_i = vel;
    float spd_i = glm::length(vel);
    float dec_factor;

    // get decceleration based on current speed and apply
    if (spd_i != 0.0f)
        dec_factor = _deccel / glm::length(vel);
    else
        dec_factor = 0.0f;
    vel -= vel * dec_factor;

    // determine if deccelerated completely ("passed" 0)
    if ((vel_i.x > 0 && vel.x < 0) || (vel_i.x < 0 && vel.x > 0))
        vel.x = 0.0f;
    if ((vel_i.y > 0 && vel.y < 0) || (vel_i.y < 0 && vel.y > 0))
        vel.y = 0.0f;

    // accelerate based on position of target
    glm::vec3 dir;
    if (target) {
        dir = glm::normalize(target->transform.pos - transform.pos);
        if (dir.x == 0.0f && dir.y == 0.0f)
            dir = _prevdir;
        else
            _prevdir = dir;

        // orient direction vector based on time
        float sint = sinf(_t / 30.0f) * 22.5f;
        dir = glm::rotate(dir, glm::radians(sint), glm::vec3(0.0f, 0.0f, 1.0f));
    } else
        dir = _prevdir;
    
    vel += glm::vec3(dir.x * _accel, dir.y * _accel, 0.0f);

    // reduce velocity to max if speed exceeds max
    if (glm::length(vel) > _spd_max)
        vel = glm::normalize(vel) * _spd_max;
}

void Enemy::enemyCollision() {
        _health--;

    // spawn 2-3 particles
    int count = (rand() % 2) + 2;
    for (int i = 0; i < count; i++) {
        //Entity *effect = getManager()->getEntity(getManager()->spawnEntity("BallParticle"));
        //effect->getQuad()->pos.v = getBox()->pos;
    }

    if (_health <= 0)
        enqueueKill();
}

// --------------------------------------------------------------------------------------------------------------------------

void Ring::_initGfxBall() {
    // display beneath other entities
    quad()->bv_pos.v.z = -1.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(64.0f, 64.0f, 0.0f)};
}

void Ring::_baseGfxBall() {
    int cyclestate = 0;

    // get first player found
    Player *p = nullptr;
    auto players = getAllProvided();
    if (players) {
        for (auto &player : *players) {
            p = player;
            break;
        }
    }

    // check player's distance from this instance's center
    if (p)
        if (glm::length(p->transform.pos - transform.pos) < 32.0f)
            cyclestate = 1;
}

void Ring::_killGfxBall() {}

Ring::Ring() : GfxBall("", -1) {}