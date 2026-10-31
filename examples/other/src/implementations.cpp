#include "implementations.hpp"

GlobalState globalstate;

const unsigned EXECUTION_QUEUES = 2;

const float diag_factor = glm::sin(glm::radians(45.0f));

GlobalState::GlobalState() : executor(EXECUTION_QUEUES, &glenv, &collisionspace), game_state(0), progress(0.0f), spawn_cooldown(120) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Enemy_Chaser::_initEntity() {
    Entity_Being* being = globalstate.resource_Being.container.get(this);
    being->hurt_cooldown_max = 4.0f;
    being->health = 3.0f;

    _entity_travel = globalstate.resource_Travel.container.get(this);
    _entity_travel->accel_speed = 0.01f;
    _entity_travel->max_speed = 0.75f;
    _entity_travel->lock(&key());

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 5.0f;
}

void Entity_Enemy_Chaser::_execEntity() {
    // if travel component is kill enqueued, unlock travel and its target; otherwise, operate as normal
    if (_entity_travel->getKillEnqueued()) {
        if (_entity_travel->target)
            _entity_travel->target->unlock(&key());
        _entity_travel->unlock(&key());
    } else {
        // if no target set, get and lock player; otherwise, check if it needs to be unlocked
        if (!_entity_travel->target) {
            _entity_travel->target = globalstate.resource_Player.container.getLast();
            if (_entity_travel->target)
                _entity_travel->target->lock(&key());
        } else {
            if (_entity_travel->target->getKillEnqueued()) {
                _entity_travel->target->unlock(&key());
                _entity_travel->target = nullptr;
            }
        }
    }
}

void Entity_Enemy_Chaser::_collideEntity(EntityInterface* other) {
    if (!getKillEnqueued() && globalstate.resource_Being.container.get(other))
        root()->enqueueKillAll();
}

Entity_Enemy_Chaser::Entity_Enemy_Chaser() : _entity_travel(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightBall::_initEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 120.0f;

    Entity_RepeatSpawn* repeatspawn = globalstate.resource_RepeatSpawn.container.get(this);
    repeatspawn->entity_name = "Entity_LightParticle";
    repeatspawn->spawnrate = 4.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 1.0f;
}

void Entity_LightBall::_collideEntity(EntityInterface* other) {
    root()->enqueueKillAll();
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightParticle::_initEntity() {
    Entity_Lifetime* lifetime = globalstate.resource_Lifetime.container.get(this);
    lifetime->lifetime = 24.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Player::_initEntity() {
    Entity_Being* being = globalstate.resource_Being.container.get(this);
    being->hurt_cooldown_max = 30.0f;
    being->health = 10.0f;

    Entity_Hitbox* hitbox = globalstate.resource_Hitbox.container.get(this);
    hitbox->damage = 1.0f;
}

void Entity_Player::_execEntity() {
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

    if (false)
        root()->enqueueKillAll();
}

Entity_Player::Entity_Player() :
    EntityInterface(),
    _shoot_cooldown_max(30.0f),
    _shoot_cooldown(0.0f),
    _speed(0.75f),
    _last_input_dir(0.0f, 1.0f, 0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Being::_execEntity() {
    if (health <= 0.0f) {
        root()->enqueueKillAll();
        return;
    }

    // check cooldowns
    if (hurt_cooldown > 0.0f)
        hurt_cooldown -= 1.0f;
}

void Entity_Being::_collideEntity(EntityInterface* other) {
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

Entity_Hitbox::Entity_Hitbox() : damage(0.0f) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Travel::_execEntity() {
    if (target) {
        glm::vec3 dir = target->globaltransform().pos - globaltransform().pos;
        if (glm::length(dir) > 0)
            dir = glm::normalize(dir);
        
        vel += dir * accel_speed;
        if (glm::length(vel) > max_speed)
            vel = glm::normalize(vel) * max_speed;
    }
    globaltransform().pos += vel;
}

Entity_Travel::Entity_Travel() : vel(0.0f), accel_speed(0.0f), max_speed(0.0f), target(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Lifetime::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (lifetime == 0)
        root()->enqueueKillAll();
}

Entity_Lifetime::Entity_Lifetime() : EntityInterface(), lifetime(0) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Pickup::_collideEntity(EntityInterface* other) {
    auto& i = globalstate.inventory;
    if (i.find(item_name) != i.end())
        i[item_name] += 1;
    root()->enqueueKillAll();
}

Entity_Pickup::Entity_Pickup() : EntityInterface(), item_name("") {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_RepeatSpawn::_execEntity() {
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