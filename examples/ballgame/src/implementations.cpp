#include "implementations.hpp"

void Bullet::_initPhysBall() {
    providers().ShrinkParticle_provider.subscribe(this);

    _i = 0;
    vel = _direction;

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(6.0f, 6.0f, 0.0f)};

    setChannel(getExecutorID());
    enableReception(true);
}

void Bullet::_basePhysBall() {
    _i++;

    if (_i % 6 == 0)
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);

    if (_i >= _lifetime || sphere()->getCollidedCount()) {
        enqueueKill();
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
    }
}

void Bullet::_killPhysBall() {
    enableReception(false);
    removeFromProvider();
}

void Bullet::_receive(ShrinkParticle *p) {
    if (!getKillEnqueued())
        p->set(glm::vec3(4.0f), glm::vec4(1.0f), 24, glm::vec3(0.0f));
    else
        p->set(transform.scale, glm::vec4(1.0f), 24, _direction / 4.0f);
};

Bullet::Bullet(Providers *providers) : PhysBall("", "Bullet"), ProvidersHolder(providers), _i(0), _lifetime(119), _direction(0.0f) {}

void Bullet::setDirection(glm::vec3 direction) { _direction = direction; }

// --------------------------------------------------------------------------------------------------------------------------

void Bomb::_initPhysBall() {
    providers().ShrinkParticle_provider.subscribe(this);
    providers().Explosion_provider.subscribe(this);

    _i = 0;
    vel = _direction * 0.5f;

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(10.0f, 10.0f, 0.0f)};

    Receiver<ShrinkParticle>::setChannel(getExecutorID());
    Receiver<ShrinkParticle>::enableReception(true);
    Receiver<Explosion>::setChannel(getExecutorID());
    Receiver<Explosion>::enableReception(true);
}

void Bomb::_basePhysBall() {
    _i++;

    if (_i % 24 == 0)
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);

    if (_i >= _lifetime || sphere()->getCollidedCount()) {
        enqueueKill();
        if (sphere()->getCollidedCount())
            executor().enqueueSpawnEntity("Explosion", 0, getExecutorID(), transform);
        else
            executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
    }
}

void Bomb::_killPhysBall() {
    Receiver<ShrinkParticle>::enableReception(false);
    Receiver<Explosion>::enableReception(false);
    removeFromProvider();
}

void Bomb::_receive(ShrinkParticle *p) {
    if (!getKillEnqueued())
        p->set(glm::vec3(8.0f), glm::vec4(1.0f), 24, glm::vec3(0.0f));
    else
        p->set(transform.scale, glm::vec4(1.0f), 24, _direction / 4.0f);
};

void Bomb::_receive(Explosion *e) {
    e->set(0.25f, 16.0f, glm::vec4(1.0f), 24, glm::vec3(0.0f), 0.029f, 0.25f, 1);
};

Bomb::Bomb(Providers *providers) : PhysBall("", "Bullet"), ProvidersHolder(providers), _i(0), _lifetime(239), _direction(0.0f) {}

void Bomb::setDirection(glm::vec3 direction) { _direction = direction; }

// --------------------------------------------------------------------------------------------------------------------------

void Explosion::_initPhysBall() {
    // display above other entities
    quad()->bv_pos.v.z = 1.0f;
    quad()->bv_color.v = _color;
}

void Explosion::_basePhysBall() {
    if (sphere()->getCollidedCount() > 0 || _i >= _active_time)
        sphere()->enableCollision(false);
    
    if (_lifetime >= 0) {
        _i++;
        if (_i >= _lifetime)
            enqueueKill();
    }

    // scale quad linearly based on rate
    if (_i % _update_rate == 0) {
        transform.pos = transform.pos + _vel;
        transform.scale = glm::vec3(_base_outerrad) + glm::vec3(float(_i) * _rate_outer);

        quad()->bv_innerrad.v = _base_innerrad + (float(_i) * _rate_inner);
    }
}

void Explosion::_killPhysBall() {
    removeFromProvider();
}

Explosion::Explosion() : PhysBall("", "Bullet"), 
    _base_innerrad(0.0f), 
    _base_outerrad(0.0f), 
    _color(glm::vec4(0.0f)), 
    _vel(glm::vec3(0.0f)), 
    _rate_inner(0.0f), 
    _rate_outer(0.0f),
    _lifetime(0),
    _i(0),
    _active_time(5),
    _update_rate(0)
{}

void Explosion::set(float base_innerrad, float base_outerrad, glm::vec4 color, unsigned lifetime, glm::vec3 vel, float rate_inner, float rate_outer, unsigned update_rate) {
    _base_innerrad = base_innerrad;
    _base_outerrad = base_outerrad;
    _color = color;
    _lifetime = lifetime;
    _vel = vel;
    _rate_inner = rate_inner;
    _rate_outer = rate_outer;
    _update_rate = update_rate;

    transform.scale = glm::vec3(_base_outerrad);
}

// --------------------------------------------------------------------------------------------------------------------------

void Player::_initPhysBall() {
    providers().Bullet_provider.subscribe(this);
    providers().Bomb_provider.subscribe(this);
    providers().ShrinkParticle_provider.subscribe(this);

    Receiver<Bullet>::setChannel(getExecutorID());
    Receiver<Bullet>::enableReception(true);
    Receiver<Bomb>::setChannel(getExecutorID());
    Receiver<Bomb>::enableReception(true);
    Receiver<ShrinkParticle>::setChannel(getExecutorID());
    Receiver<ShrinkParticle>::enableReception(true);

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    
    // override transform scale
    transform = Transform{transform.pos, glm::vec3(12.0f, 12.0f, 0.0f)};
}

void Player::_basePhysBall() {
    if (sphere()->getCollidedCount()) {
        enqueueKill();
        playerDeath();
    }

    if (_input) {
        playerMotion();
        playerAction();
    }
}

void Player::_killPhysBall() {
    Receiver<Bullet>::enableReception(false);
    Receiver<Bomb>::enableReception(false);
    Receiver<ShrinkParticle>::enableReception(false);
    removeFromProvider();
}

void Player::_receive(Bullet *bullet) { bullet->setDirection(_dirvec); }
void Player::_receive(Bomb *bomb) { bomb->setDirection(_dirvec); }
void Player::_receive(ShrinkParticle *particle) {
    // pop direction and set particle with it
    if (!_deathparticledirs.empty()) {
        glm::vec3 &dir = _deathparticledirs.front();
        particle->set(transform.scale * 1.25f, glm::vec4(1.0f), 30, dir * 0.3f);
        _deathparticledirs.pop();
    }
}

Player::Player(Providers *providers) : 
    PhysBall("", "Player"),
    ProvidersHolder(providers),
    _input(nullptr),
    _accel(0.2f), 
    _deccel(0.15f), 
    _spd_max(0.8f),
    _bullet_cooldown(0.0f),
    _bomb_cooldown(0.0f),
    _bullet_cooldown_max(15.0f),
    _bomb_cooldown_max(60.0f),
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
        _dirvec = mousepos3d - sphere()->transform.pos;
        _dirvec /= glm::length(_dirvec);
    } else
        _dirvec = glm::vec3(_prevmovedir.x, _prevmovedir.y, 0.0f);
    
    // spawn bullet if not on cooldown
    if (_bullet_cooldown <= 0.0f) {
        if (_input->get_m1()) {
            // spawn projectile and set cooldown
            executor().enqueueSpawnEntity("Bullet", 0, getExecutorID(), transform);
            _bullet_cooldown = _bullet_cooldown_max;
        }
    } else
        _bullet_cooldown -= 1.0f;

    // spawn bomb if not on cooldown
    if (_bomb_cooldown <= 0.0f) {
        if (_input->get_m2()) {
            // spawn projectile and set cooldown
            executor().enqueueSpawnEntity("Bomb", 0, getExecutorID(), transform);
            _bomb_cooldown = _bomb_cooldown_max;
        }
    } else
        _bomb_cooldown -= 1.0f;
}

void Player::playerDeath() {
    // get random count of 3-4, and random starting angle
    int count = (rand() % 2) + 3;
    glm::vec3 angle = random_angle(glm::vec3(1.0f, 0.0f, 0.0f), 180);
    for (int i = 0; i < count; i++) {
        // rotate angle by 360 / count around Z axis
        angle = glm::rotate(angle, glm::radians(360.0f / float(count)), glm::vec3(0.0f, 0.0f, 1.0f));

        // enqueue particle and store angle
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
        _deathparticledirs.push(angle);
    }
}

void Player::set(GLFWInput *input) { _input = input; }

// --------------------------------------------------------------------------------------------------------------------------

void Enemy::_initPhysBall() {
    providers().ShrinkParticle_provider.subscribe(this);
    
    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    quad()->bv_color.v = glm::vec4(0.2116f, 0.2116f, 0.2166f, 1.0f);

    Receiver<ShrinkParticle>::setChannel(getExecutorID());
    Receiver<ShrinkParticle>::enableReception(true);
}

void Enemy::_basePhysBall() {
    if (_health <= 0 || (_killflag && *_killflag)) {
        enqueueKill();
        enemyDeath();

    } else {
        if (sphere()->getCollidedCount())
            enemyCollision();
    
        enemyMotion();
    }

    _t++;
}

void Enemy::_killPhysBall() {}

void Enemy::_receive(ShrinkParticle *particle) {
    // pop direction and set particle with it
    if (!_deathparticledirs.empty()) {
        glm::vec3 &dir = _deathparticledirs.front();

        // use different values if death particles or not
        if (!getKillEnqueued())
            particle->set(transform.scale * 0.75f, glm::vec4(0.2116f, 0.2116f, 0.2166f, 1.0f), 12, dir * 1.0f);
        else
            particle->set(transform.scale * 1.25f, glm::vec4(0.2116f, 0.2116f, 0.2166f, 1.0f), 30, dir * 0.3f);
        
        _deathparticledirs.pop();
    }
}

Entity *Enemy::_getTarget() {
    // return first ID found
    auto players = providers().Player_provider.getAllProvided();
    if (players)
        for (auto &player : *players)
            return player;
    
    return nullptr;
}

Enemy::Enemy(Providers *providers) :
    PhysBall("", "Enemy"),
    ProvidersHolder(providers),
    _accel(0.075f), 
    _deccel(0.05f), 
    _spd_max(0.15f), 
    _t(rand() % 256), 
    _prevdir(0.0f),
    _health(1.0f),
    _killflag(nullptr)
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

    // only spawn damage particles if you're not going to die afterward
    if (_health > 0) {
        // get random count of 2-3, and random starting angle
        int count = (rand() % 2) + 2;
        glm::vec3 angle = random_angle(glm::vec3(1.0f, 0.0f, 0.0f), 180);
        for (int i = 0; i < count; i++) {
            // rotate angle by 360 / count around Z axis
            angle = glm::rotate(angle, glm::radians(360.0f / float(count)), glm::vec3(0.0f, 0.0f, 1.0f));

            // enqueue particle and store angle
            executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
            _deathparticledirs.push(angle);
        }
    }
}

void Enemy::enemyDeath() {
    // get random count of 3-4, and random starting angle
    int count = (rand() % 2) + 3;
    glm::vec3 angle = random_angle(glm::vec3(1.0f, 0.0f, 0.0f), 180);
    for (int i = 0; i < count; i++) {
        // rotate angle by 360 / count around Z axis
        angle = glm::rotate(angle, glm::radians(360.0f / float(count)), glm::vec3(0.0f, 0.0f, 1.0f));

        // enqueue particle and store angle
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
        _deathparticledirs.push(angle);
    }
}

void Enemy::setHealth(float health) { _health = health; }
void Enemy::setKillFlag(bool *killflag) { _killflag = killflag; }

// --------------------------------------------------------------------------------------------------------------------------

void Ring::_initGfxBall() {
    // display beneath other entities
    quad()->bv_pos.v.z = -1.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(128.0f, 128.0f, 0.0f)};
}

void Ring::_baseGfxBall() {
    quad()->bv_color.v = glm::vec4(0.4941f, 0.4941f, 0.4941f, 1.0f);

    // get first player found
    Player *p = nullptr;
    auto players = providers().Player_provider.getAllProvided();
    if (players) {
        for (auto &player : *players) {
            p = player;
            break;
        }
    }

    // check player's distance from this instance's center
    if (p)
        if (glm::length(p->transform.pos - transform.pos) < 64.0f)
            quad()->bv_color.v = glm::vec4(0.6039f, 0.6039f, 0.6039f, 1.0f);
}

void Ring::_killGfxBall() {}

Ring::Ring(Providers *providers) : GfxBall("", -1), ProvidersHolder(providers) {}

// --------------------------------------------------------------------------------------------------------------------------

void ShrinkParticle::_initGfxBall() {
    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    quad()->bv_color.v = _color;
}

void ShrinkParticle::_baseGfxBall() {
    // scale quad linearly with lifetime
    transform.pos = transform.pos + _vel;
    transform.scale = _basescale * ((_lifetime - float(_i)) / _lifetime);
}

void ShrinkParticle::_killGfxBall() {
    removeFromProvider();
}

ShrinkParticle::ShrinkParticle() : GfxBall("", 0), _basescale(glm::vec3(0.0f)), _color(glm::vec4(0.0f)), _vel(glm::vec3(0.0f)) {}

void ShrinkParticle::set(glm::vec3 basescale, glm::vec4 color, unsigned lifetime, glm::vec3 vel) {
    _basescale = basescale;
    _color = color;
    _lifetime = lifetime;
    _vel = vel;

    transform.scale = _basescale;
}