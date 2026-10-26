#include "implementations.hpp"

GlobalState globalstate;

const unsigned EXECUTION_QUEUES = 2;

const float diag_factor = glm::sin(glm::radians(45.0f));

GlobalState::GlobalState() : 
    executor(EXECUTION_QUEUES, &glenv, &collisionspace)
{
    allocator_Player.provider().attach(&globalstate.container_Player);
    allocator_Being.provider().attach(&globalstate.container_Being);
    allocator_Hitbox.provider().attach(&globalstate.container_Hitbox);
    allocator_Travel.provider().attach(&globalstate.container_Travel);
    allocator_Lifetime.provider().attach(&globalstate.container_Lifetime);
    allocator_Pickup.provider().attach(&globalstate.container_Pickup);
    allocator_RepeatSpawn.provider().attach(&globalstate.container_RepeatSpawn);
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Enemy_Chaser::_initEntity() {
    Entity_Being* being = globalstate.container_Being.getNode(this);
    being->hurt_cooldown_max = 4.0f;
    being->health = 3.0f;

    _entity_travel = globalstate.container_Travel.getNode(this);
    _entity_travel->vel = glm::vec3(speed, 0.0f, 0.0f);
    _entity_travel->lock(&key());

    Entity_Hitbox* hitbox = globalstate.container_Hitbox.getNode(this);
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
            _entity_travel->target = globalstate.container_Player.getLastNode();
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
    if (!getKillEnqueued() && globalstate.container_Being.getNode(other))
        root()->enqueueKillAll();
}

Entity_Enemy_Chaser::Entity_Enemy_Chaser() : _entity_travel(nullptr), speed(0.5f) {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightBall::_initEntity() {
    Entity_Lifetime* lifetime = globalstate.container_Lifetime.getNode(this);
    lifetime->lifetime = 1.0f;

    Entity_RepeatSpawn* repeatspawn = globalstate.container_RepeatSpawn.getNode(this);
    repeatspawn->entity_name = "Entity_LightParticle";
    repeatspawn->spawnrate = 4.0f;

    Entity_Travel* travel = globalstate.container_Travel.getNode(this);
    travel->vel = glm::vec3(1.0f, 0.0f, 0.0f);

    Entity_Hitbox* hitbox = globalstate.container_Hitbox.getNode(this);
    hitbox->damage = 1.0f;
}

void Entity_LightBall::_collideEntity(EntityInterface* other) {
    root()->enqueueKillAll();
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightParticle::_initEntity() {
    Entity_Lifetime* lifetime = globalstate.container_Lifetime.getNode(this);
    lifetime->lifetime = 24.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Player::_initEntity() {
    Entity_Being* being = globalstate.container_Being.getNode(this);
    being->hurt_cooldown_max = 30.0f;
    being->health = 10.0f;

    Entity_Hitbox* hitbox = globalstate.container_Hitbox.getNode(this);
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
        _shoot_cooldown = _shoot_cooldown_max;

        globalstate.executor.spawnEntity("Entity_LightBall", globaltransform());
        Entity_Lifetime* ball = globalstate.container_Lifetime.getLastNode();
        ball->lifetime = 90.0f;
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
    Entity_Hitbox* hitbox = globalstate.container_Hitbox.getNode(other);
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
            dir = glm::normalize(dir) * glm::length(vel);
        globaltransform().pos += dir;
    } else
        globaltransform().pos += vel;
}

Entity_Travel::Entity_Travel() : vel(0.0f), target(nullptr) {}

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

            Transform spawn_transform = globaltransform();
            spawn_transform.pos = spawn_transform.pos + glm::vec3(0.0f, (float(rand() % 50) / 10.0f) - 2.5f, 0.0f);
            globalstate.executor.spawnEntity(entity_name.c_str(), spawn_transform);
        }
    }
}

Entity_RepeatSpawn::Entity_RepeatSpawn() : EntityInterface(), spawnrate(-1), spawnrate_cooldown(0), entity_name("") {}