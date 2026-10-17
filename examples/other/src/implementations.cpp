#include "implementations.hpp"

GlobalState globalstate;

const unsigned EXECUTION_QUEUES = 2;

const float diag_factor = glm::sin(glm::radians(45.0f));

GlobalState::GlobalState() : 
    executor(EXECUTION_QUEUES, &glenv, &collisionspace)
{
    allocator_Player.provider().attach(&globalstate.container_Player);
    allocator_Lifetime.provider().attach(&globalstate.container_Lifetime);
    allocator_Pickup.provider().attach(&globalstate.container_Pickup);
    allocator_RepeatSpawn.provider().attach(&globalstate.container_RepeatSpawn);
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightBall::_initEntity() {
    Entity_RepeatSpawn* repeatspawn = globalstate.container_RepeatSpawn.getInstance(entities()[1]);
    repeatspawn->entity_name = "Entity_LightParticle";
    repeatspawn->lifetime = 90.0f;
    repeatspawn->spawnrate = 4.0f;
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_LightParticle::_initEntity() {
    Entity_Lifetime* particle = globalstate.container_Lifetime.getInstance(entities()[0]);
    particle->lifetime = 24.0f;
    particle->vel = glm::vec3(-1.0f * (float(rand() % 5) / 10.0f), 0.0f, 0.0f);
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Player::_execEntity() {
    // check cooldowns
    if (_hurt_cooldown > 0.0f)
        _hurt_cooldown -= 1.0f;
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
        Entity_Lifetime* ball = globalstate.container_Lifetime.getLastInstance();
        ball->lifetime = 90.0f;
        ball->vel = glm::vec3(1.75f, 0.0f, 0.0f);
    }

    pos += vel;

    if (false)
        startKillAll();
}
void Entity_Player::_collide(EntityInterface* other) {
    _hurt_cooldown = _hurt_cooldown_max;
}

Entity_Player::Entity_Player() :
    EntityInterface(),
    _hurt_cooldown_max(120.0f),
    _hurt_cooldown(0.0f),
    _shoot_cooldown_max(30.0f),
    _shoot_cooldown(0.0f),
    _speed(0.75f),
    _last_input_dir(0.0f, 1.0f, 0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void BeingInterface::_initEntity() {
    _initBeing();
}

void BeingInterface::_execEntity() {
    _execBeing();

    if (health <= 0.0f) {
        startKillAll();
        return;
    }

    // check cooldowns
    if (hurt_cooldown > 0.0f)
        hurt_cooldown -= 1.0f;
}

void BeingInterface::_killEntity() {
    _killBeing();
}

void BeingInterface::_collide(EntityInterface* other) {
    _collideBeing(other);
    if (hurt_cooldown <= 0.0f)
        hurt_cooldown = hurt_cooldown_max;
}

void BeingInterface::_initBeing() {}
void BeingInterface::_execBeing() {}
void BeingInterface::_killBeing() {}
void BeingInterface::_collideBeing(EntityInterface* other) {}

BeingInterface::BeingInterface() :
    EntityInterface(),
    hurt_cooldown(0.0f),
    hurt_cooldown_max(120.0f),
    health(0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Lifetime::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (_target)
        globaltransform().pos = _target->globaltransform().pos;
    else
        globaltransform().pos += vel;
    
    if (lifetime == 0)
        startKillAll();
}
void Entity_Lifetime::_collide(EntityInterface* other) {
    startKillAll();
}

Entity_Lifetime::Entity_Lifetime() : EntityInterface(), _target(nullptr), lifetime(0), vel(glm::vec3(0.0f)) {}

void Entity_Lifetime::setTarget(EntityInterface* target) {
    _target = target;
    if (_target)
        _target->attachNullableRef(&_target);
}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_Pickup::_collide(EntityInterface* other) {
    auto& i = globalstate.inventory;
    if (i.find(item_name) != i.end())
        i[item_name] += 1;
    startKillAll();
}

Entity_Pickup::Entity_Pickup() : EntityInterface(), item_name("") {}

// --------------------------------------------------------------------------------------------------------------------------

void Entity_RepeatSpawn::_execEntity() {
    if (lifetime > 0)
        lifetime--;
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
    
    if (lifetime == 0)
        startKillAll();
}

Entity_RepeatSpawn::Entity_RepeatSpawn() : EntityInterface(), lifetime(0), spawnrate(-1), spawnrate_cooldown(0), entity_name("") {}