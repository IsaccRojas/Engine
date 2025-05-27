#include "implementations.hpp"

// set health of created enemies
void GlobalState::_receive(Enemy *enemy) {
    if (!size_factors.empty()) {
        int size_factor = size_factors.front();
        enemy->set(1.0f + float(size_factor));
        size_factors.pop();
    }
}

// set upgrade for upgraders
void GlobalState::_receive(Upgrader *upgrader) {
    if (!upgrade_indices.empty()) {
        int upgrade_index = upgrade_indices.front();
        upgrader->set(upgrade_index);
        upgrade_indices.pop();
    }
}

GlobalState::GlobalState(GLEnv *glenv, GLFWInput *glfwinput) : input(glfwinput) {
    toptext.setEnv(glenv);
    subtext.setEnv(glenv);
    bottomtext.setEnv(glenv);
    pointstext.setEnv(glenv);
    timetext.setEnv(glenv);

    reset();
    Receiver<Enemy>::setChannel(65536);
    Receiver<Enemy>::enableReception(true);
    Receiver<Upgrader>::setChannel(65536);
    Receiver<Upgrader>::enableReception(true);
}

GlobalState::~GlobalState() {
    upgrade_texts.clear();
}

void GlobalState::reset() {
    prev_time = std::chrono::steady_clock::now();

    /* 
    0: game start
    1: round active
    2: round inactive (complete)
    3: round inactive (failed)
    */
    game_state = 0;
    i = 0;
    time = 0;
    number = 0.0f;
    rate = 0.0f;
    target_number = 325.0f;
    max_rate = 0.09375f;
    rate_increase = 0.00035f;
    rate_decrease = -0.0007f;
    round = 1;
    difficulty = 1;
    spawn_rate = 1;
    enter_check = false;
    enter_state = false;
    killenemyflag = true;
    killupgraderflag = true;
    /*
        0 - primary fire rate up
        1 - primary damage up
        2 - primary count up
        3 - bomb fire rate up
        4 - bomb damage up
        5 - bomb radius up
    */
    upgrade_prices = std::vector<int>(6, 0);
    upgrade_prices[0] = 35;
    upgrade_prices[1] = 87;
    upgrade_prices[2] = 105;
    upgrade_prices[3] = 35;
    upgrade_prices[4] = 87;
    upgrade_prices[5] = 52;
    upgrade_counts = std::vector<int>(6, 0);
    upgrades_spawned = false;
    points = 0;
}

void GlobalState::transition(int state) {
    // from start to active
    if (game_state == 0 && state == 1) {
        killenemyflag = false;
        killupgraderflag = true;
    }

    // from active to complete
    else if (game_state == 1 && state == 2) {
        rate = 0.0f;
        killenemyflag = true;
        killupgraderflag = false;
    }

    // from active to fail
    else if (game_state == 1 && state == 3) {
        rate = 0.0f;
        killenemyflag = false;
        killupgraderflag = true;
    }

    // from complete to active
    else if (game_state == 2 && state == 1) {
        // disable killflag, reset number and rate, increase round count and spawn rate
        killenemyflag = false;
        killupgraderflag = true;

        upgrades_spawned = false;

        number = 0.0f;
        rate = 0.0f;
        round += 1;
        spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(round)))), 5, 60);
    }

    // from fail to start
    else if (game_state == 3 && state == 0) {
        // enable killflag, reset all values
        killenemyflag = true;
        killupgraderflag = true;
        reset();
    }

    /* any other transition is undefined behavior */

    game_state = state;
}

// --------------------------------------------------------------------------------------------------------------------------

void Bullet::_initPhysBall() {
    globalstate().providers.ShrinkParticle_provider.subscribe(this);

    _i = 0;
    vel = _direction;

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(6.0f, 6.0f, 0.0f)};

    setChannel(getExecutorID());
    enableReception(true);

    // set weight
    sphere()->attributes["weight"] = 1.0f;
}

void Bullet::_basePhysBall() {
    _i++;

    if (_i % 6 == 0)
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);

    if (_i >= _lifetime || health <= 0.0f) {     
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
        enqueueKill();
    }
}

void Bullet::_killPhysBall() {
    enableReception(false);
    removeFromProvider();
}

void Bullet::_onCollision(Sphere *other) {
    health--;
    if (health <= 0.0f)
        sphere()->enableCollision(false);
};

void Bullet::_receive(ShrinkParticle *p) {
    if (!getKillEnqueued())
        p->set(glm::vec3(4.0f), glm::vec4(1.0f), 24, glm::vec3(0.0f));
    else
        p->set(transform.scale, glm::vec4(1.0f), 24, _direction / 4.0f);
};

Bullet::Bullet(GlobalState *globalstate) : PhysBall("", "Bullet"), StateReferrer(globalstate), _i(0), _lifetime(119), _direction(0.0f) {}

void Bullet::set(glm::vec3 direction, float health) { 
    _direction = direction;
    PhysBall::health = health;
}

// --------------------------------------------------------------------------------------------------------------------------

void Bomb::_initPhysBall() {
    globalstate().providers.ShrinkParticle_provider.subscribe(this);
    globalstate().providers.Explosion_provider.subscribe(this);

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

    // set weight
    sphere()->attributes["weight"] = 1.0f;
}

void Bomb::_basePhysBall() {
    _i++;

    if (_i % 24 == 0)
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);

    if (sphere()->getCollidedCount()) {
        executor().enqueueSpawnEntity("Explosion", 0, getExecutorID(), transform);
        enqueueKill();

    } else if (_i >= _lifetime) {
        executor().enqueueSpawnEntity("ShrinkParticle", 1, getExecutorID(), transform);
        enqueueKill();
    }
}

void Bomb::_killPhysBall() {
    Receiver<ShrinkParticle>::enableReception(false);
    Receiver<Explosion>::enableReception(false);
    removeFromProvider();
}

void Bomb::_onCollision(Sphere *other) {
    sphere()->enableCollision(false);
};

void Bomb::_receive(ShrinkParticle *p) {
    if (!getKillEnqueued())
        p->set(glm::vec3(8.0f), glm::vec4(1.0f), 24, glm::vec3(0.0f));
    else
        p->set(transform.scale, glm::vec4(1.0f), 24, _direction / 4.0f);
};

void Bomb::_receive(Explosion *e) {
    e->set(0.25f, 16.0f + (4.0f * globalstate().upgrade_counts[5]), glm::vec4(1.0f), 16, glm::vec3(0.0f), 0.045f, 0.25f, 2, 1 + globalstate().upgrade_counts[4]);
};

Bomb::Bomb(GlobalState *globalstate) : PhysBall("", "Bullet"), StateReferrer(globalstate), _i(0), _lifetime(239), _direction(0.0f) {}

void Bomb::set(glm::vec3 direction) { _direction = direction; }

// --------------------------------------------------------------------------------------------------------------------------

void Explosion::_initPhysBall() {
    // display above other entities
    quad()->bv_pos.v.z = 1.0f;
    quad()->bv_color.v = _color;

    // set weight
    sphere()->attributes["weight"] = _damage;
}

void Explosion::_basePhysBall() {
    // only allow collision for one frame
    if (_i > 0)
        sphere()->enableCollision(false);

    // scale quad linearly based on rate
    if (_i % _update_rate == 0) {
        transform.pos = transform.pos + _vel;
        transform.scale = glm::vec3(_base_outerrad) + glm::vec3(float(_i) * _rate_outer);

        quad()->bv_innerrad.v = _base_innerrad + (float(_i) * _rate_inner);
    }

    _i++;
    if (_i >= _lifetime)
        enqueueKill();
}

void Explosion::_killPhysBall() {
    removeFromProvider();
}

void Explosion::_onCollision(Sphere *other) {}

Explosion::Explosion() : PhysBall("", "Bullet"), 
    _base_innerrad(0.0f), 
    _base_outerrad(0.0f), 
    _color(glm::vec4(0.0f)), 
    _vel(glm::vec3(0.0f)), 
    _rate_inner(0.0f), 
    _rate_outer(0.0f),
    _lifetime(1),
    _i(0),
    _update_rate(0),
    _damage(0.0f)
{}

void Explosion::set(float base_innerrad, float base_outerrad, glm::vec4 color, unsigned lifetime, glm::vec3 vel, float rate_inner, float rate_outer, unsigned update_rate, float damage) {
    _base_innerrad = base_innerrad;
    _base_outerrad = base_outerrad;
    _color = color;
    _lifetime = lifetime;
    _vel = vel;
    _rate_inner = rate_inner;
    _rate_outer = rate_outer;
    _update_rate = update_rate;
    _damage = damage;

    transform.scale = glm::vec3(_base_outerrad);
}

// --------------------------------------------------------------------------------------------------------------------------

void Player::_initPhysBall() {
    globalstate().providers.Bullet_provider.subscribe(this);
    globalstate().providers.Bomb_provider.subscribe(this);
    globalstate().providers.ShrinkParticle_provider.subscribe(this);

    Receiver<Bullet>::setChannel(getExecutorID());
    Receiver<Bullet>::enableReception(true);
    Receiver<Bomb>::setChannel(getExecutorID());
    Receiver<Bomb>::enableReception(true);
    Receiver<ShrinkParticle>::setChannel(getExecutorID());
    Receiver<ShrinkParticle>::enableReception(true);

    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    
    // override transform scale
    transform = Transform{transform.pos, glm::vec3(1.0f, 1.0f, 0.0f)};

    // set weight
    sphere()->attributes["weight"] = 1.0f;
}

void Player::_basePhysBall() {
    // "growing" animation when first spawned
    if (transform.scale.x < 12.0f)
        transform.scale = transform.scale + glm::vec3(1.0f, 1.0f, 0.0f);

    if (sphere()->getCollidedCount()) {
        playerDeath();
        enqueueKill();
    }

    if (globalstate().input) {
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

void Player::_onCollision(Sphere *other) {
    sphere()->enableCollision(false);
}

void Player::_receive(Bullet *bullet) {
    // pop direction and set particle with it
    if (!_bulletdirs.empty()) {
        glm::vec3 &dir = _bulletdirs.front();
        bullet->set(dir, 1.0f + globalstate().upgrade_counts[1]);
        _bulletdirs.pop();
    }    
}
void Player::_receive(Bomb *bomb) { bomb->set(_dirvec); }
void Player::_receive(ShrinkParticle *particle) {
    // pop direction and set particle with it
    if (!_deathparticledirs.empty()) {
        glm::vec3 &dir = _deathparticledirs.front();
        particle->set(transform.scale * 1.25f, glm::vec4(1.0f), 30, dir * 0.3f);
        _deathparticledirs.pop();
    }
}

Player::Player(GlobalState *globalstate) : 
    PhysBall("", "Player"),
    StateReferrer(globalstate),
    _accel(0.2f), 
    _deccel(0.15f), 
    _spd_max(0.8f),
    _bullet_cooldown(0.0f),
    _bomb_cooldown(0.0f),
    _bullet_cooldown_max(30.0f),
    _bomb_cooldown_max(75.0f),
    _prevmovedir(0.0f),
    _dirvec(0.0f, -1.0f, 0.0f)
{}

void Player::playerMotion() {
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

    // get WASD and left stick input
    glm::vec2 dir = globalstate().input->inputdir();
    glm::vec2 leftstick = globalstate().input->leftstick();
    if (glm::length(leftstick) > 0.25f)
        dir += leftstick;

    // normalize and accelerate
    if (glm::length(dir) > 1.0f)
        dir = glm::normalize(dir);
    vel += glm::vec3(dir.x * _accel, dir.y * _accel, 0.0f);

    // reduce velocity to max if speed exceeds max
    if (glm::length(vel) > _spd_max)
        vel = glm::normalize(vel) * _spd_max;
    
    if (glm::length(dir) > 0)
        _prevmovedir = dir;
}

void Player::playerAction() {
    if (globalstate().input->has_joystick()) {
        // get right stick
        glm::vec2 rightstick = globalstate().input->rightstick();
        glm::vec3 rightstick_3d = glm::vec3(rightstick.x, rightstick.y, 0.0f);

        if (glm::length(rightstick_3d) > 0.25f)
            _dirvec = glm::normalize(rightstick_3d);

    } else {
        // get mouse position relative to player position
        glm::vec2 mousepos = globalstate().input->mousepos();
        glm::vec3 mousepos_3d = glm::vec3(mousepos.x, mousepos.y, 0.0f);
        glm::vec3 final_pos = mousepos_3d - sphere()->transform.pos;

        if (glm::length(final_pos) > 0.0f)
            _dirvec = glm::normalize(final_pos);
    }
    
    // normalize
    if (glm::length(_dirvec) > 1.0f)
        _dirvec = glm::normalize(_dirvec);

    // spawn bullet if not on cooldown
    if (_bullet_cooldown <= 0.0f) {
        if (globalstate().input->get_m1() || globalstate().input->get_rightbumper()) {
            float additional_count = globalstate().upgrade_counts[2];
            float arc_segment = 4.0f;
            float arc_length = 45.0f;
            if (arc_segment * additional_count >= arc_length)
                arc_segment = arc_length / additional_count;
            float arc_start = -0.5f * (additional_count * arc_segment);

            for (int i = 0; i < 1 + additional_count; i++) {
                // spawn projectile
                executor().enqueueSpawnEntity("Bullet", 0, getExecutorID(), transform);
                _bulletdirs.push(glm::rotate(_dirvec, glm::radians(arc_start + (arc_segment * i)), glm::vec3(0.0f, 0.0f, 1.0f)));
            }

            // set cooldown
            _bullet_cooldown = _bullet_cooldown_max * pow(0.95, globalstate().upgrade_counts[0]);
        }
    } else
        _bullet_cooldown -= 1.0f;

    // spawn bomb if not on cooldown
    if (_bomb_cooldown <= 0.0f) {
        if (globalstate().input->get_m2() || globalstate().input->get_leftbumper()) {
            // spawn projectile and set cooldown
            executor().enqueueSpawnEntity("Bomb", 0, getExecutorID(), transform);
            _bomb_cooldown = _bomb_cooldown_max * pow(0.95, globalstate().upgrade_counts[3]);
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

// --------------------------------------------------------------------------------------------------------------------------

void Enemy::_initPhysBall() {
    globalstate().providers.ShrinkParticle_provider.subscribe(this);
    
    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    quad()->bv_color.v = glm::vec4(0.2116f, 0.2116f, 0.2166f, 1.0f);

    Receiver<ShrinkParticle>::setChannel(getExecutorID());
    Receiver<ShrinkParticle>::enableReception(true);
}

void Enemy::_basePhysBall() {
    if (globalstate().killenemyflag || health <= 0.0f) {
        enemyDeath();
        enqueueKill();
    
    } else {
        if (sphere()->getCollidedCount())
            enemyCollision();
        
        enemyMotion();
    }

    _t++;
}

void Enemy::_killPhysBall() {
    Receiver<ShrinkParticle>::enableReception(false);
    removeFromProvider();
}

void Enemy::_onCollision(Sphere *other) {
    health -= other->attributes["weight"];
    if (health <= 0.0f) {
        globalstate().points += _max_health;
        sphere()->enableCollision(false);
    }
}

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
    auto players = globalstate().providers.Player_provider.getAllProvided();
    if (players)
        for (auto &player : *players)
            return player;
    
    return nullptr;
}

Enemy::Enemy(GlobalState *globalstate) :
    PhysBall("", "Enemy"),
    StateReferrer(globalstate),
    _accel(0.075f), 
    _deccel(0.05f), 
    _spd_max(0.15f), 
    _t(rand() % 256), 
    _prevdir(0.0f),
    _max_health(1.0f)
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
    // only spawn damage particles if you're not going to die afterward
    if (health > 0) {
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

void Enemy::set(float health) {
    PhysBall::health = health;
    _max_health = health;
}

// --------------------------------------------------------------------------------------------------------------------------

void Ring::_initGfxEntity() {
    // display beneath other entities
    quad()->bv_pos.v.z = -1.0f;

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(128.0f, 128.0f, 0.0f)};
}

void Ring::_baseGfxEntity() {
    quad()->bv_color.v = glm::vec4(0.4941f, 0.4941f, 0.4941f, 1.0f);

    // get first player found
    Player *p = nullptr;
    auto players = globalstate().providers.Player_provider.getAllProvided();
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

void Ring::_killGfxEntity() {}

Ring::Ring(GlobalState *globalstate) : GfxEntity("", -1, GLE_ELLIPSE), StateReferrer(globalstate) {}

// --------------------------------------------------------------------------------------------------------------------------

void ShrinkParticle::_initGfxEntity() {
    // display on level with other entities
    quad()->bv_pos.v.z = 0.0f;
    quad()->bv_color.v = _color;
}

void ShrinkParticle::_baseGfxEntity() {
    // scale quad linearly with lifetime
    transform.pos = transform.pos + _vel;
    transform.scale = _basescale * ((_lifetime - float(_i)) / _lifetime);
}

void ShrinkParticle::_killGfxEntity() {
    removeFromProvider();
}

ShrinkParticle::ShrinkParticle() : GfxEntity("", 0, GLE_ELLIPSE), _basescale(glm::vec3(0.0f)), _color(glm::vec4(0.0f)), _vel(glm::vec3(0.0f)) {}

void ShrinkParticle::set(glm::vec3 basescale, glm::vec4 color, unsigned lifetime, glm::vec3 vel) {
    _basescale = basescale;
    _color = color;
    _lifetime = lifetime;
    _vel = vel;

    transform.scale = _basescale;
}

// --------------------------------------------------------------------------------------------------------------------------

void Upgrader::_initGfxEntity() {
    // display above other entities
    quad()->bv_pos.v.z = 1.0f;
    quad()->animationstate().setCycleState(_upgrade_index);

    // override transform scale
    transform = Transform{transform.pos, glm::vec3(14.0f, 14.0f, 0.0f)};

    _pricetext.setEnv(&(executor().glenv()));
    _pricetext.setTextConfig(TextConfig{0, 0, 2, 5, 19, 5, 10, 0, 0, 1});
    _pricetext.setText(std::to_string(_upgrade_price).c_str());
    _pricetext.setPos(transform.pos - glm::vec3(0.0f, 13.0f, 0.0f));
    _pricetext.writeText();
}

void Upgrader::_baseGfxEntity() {
    if (_cooldown <= 0) {
        // get first player found
        Player *p = nullptr;
        auto players = globalstate().providers.Player_provider.getAllProvided();
        if (players) {
            for (auto &player : *players) {
                p = player;
                break;
            }
        }

        // check player's distance from this instance's center (if not on cooldown)
        if (p) {
            if (glm::length(p->transform.pos - transform.pos) < 20.0f) {
                if ((globalstate().input->get_e() || globalstate().input->get_button_a()) && globalstate().points >= _upgrade_price) {
                    globalstate().upgrade_counts[_upgrade_index]++;
                    _cooldown = _max_cooldown;
                    globalstate().points -= _upgrade_price;

                    enqueueKill();
                }
            }
        }
    }

    if (_cooldown > 0)
        _cooldown--;

    if (globalstate().killupgraderflag)
        enqueueKill();
}

void Upgrader::_killGfxEntity() {
    removeFromProvider();
}

Upgrader::Upgrader(GlobalState *globalstate) : GfxEntity("Upgrade", -1, GLE_RECT), StateReferrer(globalstate), _upgrade_index(0), _upgrade_price(0), _cooldown(0), _max_cooldown(45) {}

void Upgrader::set(int upgrade_index) {
    _upgrade_index = upgrade_index;
    _upgrade_price = globalstate().upgrade_prices[upgrade_index] + int(globalstate().time / 300.0f);
}