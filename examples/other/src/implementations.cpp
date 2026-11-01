#include "implementations.hpp"

GlobalState globalstate;

const unsigned EXECUTION_QUEUES = 2;

const float diag_factor = glm::sin(glm::radians(45.0f));

GlobalState::GlobalState() : executor(EXECUTION_QUEUES, &glenv, &collisionspace), game_state(0), progress(0.0f), spawn_cooldown(180) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_FireParticle::_onInitEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 16.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_FireBall::_onInitEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 720.0f;

    Entity_RepeatSpawn* repeatspawn = globalstate.resource_RepeatSpawn.container.get(this);
    repeatspawn->entity_name = "Entity_FireParticle";
    repeatspawn->spawnrate = 3.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 1.0f;
}

void Entity_FireBall::_onCollisionEntity(EntityInterface* other) {
    root()->enqueueKillAll();
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Shooter::_onInitEntity() {
    Entity_Being* being = globalstate.resource_Being.container.get(this);
    being->hurt_cooldown_max = 4.0f;
    being->health = 4.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 5.0f;

    t = globalstate.resource_Travel.container.get(this);
    t->accel_speed = 0.001f;
    t->max_speed = 0.25f;
    t->target = true;

    aq = globalstate.resource_AcquirePlayer.container.get(this);
    aq->lock(&key());
}

void Entity_Shooter::_onExecEntity() {
    if (aq->player)
        t->pos = globaltransform().pos - glm::vec3(0.0f, globaltransform().pos.y - aq->player->globaltransform().pos.y, 0.0f);

    // check cooldown
    if (_shoot_cooldown > 0.0f)
        _shoot_cooldown -= 1.0f;

    if (_shoot_cooldown <= 0.0f) {
        EntityInterface* fireball = globalstate.executor.spawnEntity("Entity_FireBall", globaltransform());
        globalstate.resource_Travel.container.get(fireball)->vel = randomAngle(glm::vec3(-0.25f, 0.0f, 0.0f), 30.0f);
        _shoot_cooldown = _shoot_cooldown_max;
    }
}

void Entity_Shooter::_onKillEnqueueEntity() {
    unlockAll();
}

void Entity_Shooter::_onCollisionEntity(EntityInterface* other) {
    if (!getKillEnqueued() && globalstate.resource_Being.container.get(other))
        root()->enqueueKillAll();
}

Entity_Shooter::Entity_Shooter() : EntityInterface(), _shoot_cooldown_max(360.0f), _shoot_cooldown(360.0f), t(nullptr), aq(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Chaser::_onInitEntity() {
    Entity_Being* being = globalstate.resource_Being.container.get(this);
    being->hurt_cooldown_max = 4.0f;
    being->health = 3.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 5.0f;

    t = globalstate.resource_Travel.container.get(this);
    t->accel_speed = 0.01f;
    t->max_speed = 0.75f;
    t->target = true;

    aq = globalstate.resource_AcquirePlayer.container.get(this);
    aq->lock(&key());
}

void Entity_Chaser::_onExecEntity() {
    if (aq->player)
        t->pos = aq->player->globaltransform().pos;
}

void Entity_Chaser::_onKillEnqueueEntity() {
    unlockAll();
}

void Entity_Chaser::_onCollisionEntity(EntityInterface* other) {
    if (!getKillEnqueued() && globalstate.resource_Being.container.get(other))
        root()->enqueueKillAll();
}

Entity_Chaser::Entity_Chaser() : EntityInterface(), t(nullptr), aq(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightBall::_onInitEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 120.0f;

    Entity_RepeatSpawn* repeatspawn = globalstate.resource_RepeatSpawn.container.get(this);
    repeatspawn->entity_name = "Entity_LightParticle";
    repeatspawn->spawnrate = 4.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 1.0f;
}

void Entity_LightBall::_onCollisionEntity(EntityInterface* other) {
    root()->enqueueKillAll();
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightParticle::_onInitEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 24.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Player::_onInitEntity() {
    Entity_Being* being = globalstate.resource_Being.container.get(this);
    being->hurt_cooldown_max = 30.0f;
    being->health = 20.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 1.0f;
}

void Entity_Player::_onExecEntity() {
    // check cooldown
    if (_shoot_cooldown > 0.0f)
        _shoot_cooldown -= 1.0f;

    glm::vec3 &pos = globaltransform().pos;

    // get velocity as sum of input directions
    glm::vec3 vel = glm::vec3(
        float(-1.0f * globalstate.input.get_a()) + float(globalstate.input.get_d()),
        float(-1.0f * globalstate.input.get_s()) + float(globalstate.input.get_w()),
        0.0f
    );
    
    // normalize velocity length to speed and update last input direction, if non-zero length
    if (glm::length(vel)) {
        _last_input_dir = vel;
        vel = _speed * glm::normalize(vel);
    }

    if (_shoot_cooldown <= 0.0f && globalstate.input.get_space()) {
        EntityInterface* b1 = globalstate.executor.spawnEntity("Entity_LightBall", globaltransform());
        globalstate.resource_Travel.container.get(b1)->vel = glm::vec3(1.25f, 0.0f, 0.0f);

        EntityInterface* b2 = globalstate.executor.spawnEntity("Entity_SmallLightBall", globaltransform());
        globalstate.resource_Travel.container.get(b2)->vel = glm::rotate(glm::vec3(1.25f, 0.0f, 0.0f), glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        
        EntityInterface* b3 = globalstate.executor.spawnEntity("Entity_SmallLightBall", globaltransform());
        globalstate.resource_Travel.container.get(b3)->vel = glm::rotate(glm::vec3(1.25f, 0.0f, 0.0f), glm::radians(-30.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        _shoot_cooldown = _shoot_cooldown_max;
    }

    pos += vel;
}

Entity_Player::Entity_Player() :
    EntityInterface(),
    _shoot_cooldown_max(30.0f),
    _shoot_cooldown(0.0f),
    _speed(0.75f),
    _last_input_dir(0.0f, 1.0f, 0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Being::_onExecEntity() {
    if (health <= 0.0f) {
        root()->enqueueKillAll();
        return;
    }

    // check cooldowns
    if (hurt_cooldown > 0.0f)
        hurt_cooldown -= 1.0f;
}

void Entity_Being::_onCollisionEntity(EntityInterface* other) {
    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(other);
    if (hitbox) {
        if (hurt_cooldown <= 0.0f) {
            health -= hitbox->damage;
            hurt_cooldown = hurt_cooldown_max;
        }
    }
}

Entity_Being::Entity_Being() :
    EntityInterface(),
    hurt_cooldown(0.0f),
    hurt_cooldown_max(120.0f),
    health(0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

Entity_Hitbox::Entity_Hitbox() : EntityInterface(), damage(0.0f) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_AcquirePlayer::_onExecEntity() {
    if (!player) {
        unsigned s = globalstate.resource_Player.container.size();
        if (s > 0) {
            auto iter = globalstate.resource_Player.container.begin();
            std::advance(iter, rand() % s);
            if (!iter->second->getKillEnqueued()) {
                player = iter->second;
                player->lock(&key());
            }        
        }

    } else {
        if (player->getKillEnqueued()) {
            player->unlock(&key());
            player = nullptr;
        }
    }
}

Entity_AcquirePlayer::Entity_AcquirePlayer() : EntityInterface(), player(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Travel::_onExecEntity() {
    if (target) {
        glm::vec3 dir = pos - globaltransform().pos;
        if (glm::length(dir) > 0)
            dir = glm::normalize(dir);
        
        vel += dir * accel_speed;
        if (glm::length(vel) > max_speed)
            vel = glm::normalize(vel) * max_speed;
    }
    globaltransform().pos += vel;
}

Entity_Travel::Entity_Travel() : EntityInterface(), vel(0.0f), accel_speed(0.0f), max_speed(0.0f), target(false), pos(0.0f) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Lifetime::_onExecEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (lifetime == 0)
        root()->enqueueKillAll();
}

Entity_Lifetime::Entity_Lifetime() : EntityInterface(), lifetime(0) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Pickup::_onCollisionEntity(EntityInterface* other) {
    auto& i = globalstate.inventory;
    if (i.find(item_name) != i.end())
        i[item_name] += 1;
    root()->enqueueKillAll();
}

Entity_Pickup::Entity_Pickup() : EntityInterface(), item_name("") {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_RepeatSpawn::_onExecEntity() {
    if (spawnrate_cooldown > 0)
        spawnrate_cooldown--;
    
    if (spawnrate >= 0 && entity_name != "") {
        if (spawnrate_cooldown == 0) {
            spawnrate_cooldown = spawnrate;
            globalstate.executor.spawnEntity(entity_name.c_str(), globaltransform());
        }
    }
}

Entity_RepeatSpawn::Entity_RepeatSpawn() : EntityInterface(), spawnrate(-1), spawnrate_cooldown(0), entity_name("") {}