#include "implementations.hpp"

GlobalState globalstate;

const unsigned EXECUTION_QUEUES = 2;

const float diag_factor = glm::sin(glm::radians(45.0f));

GlobalState::GlobalState() : 
    executor(EXECUTION_QUEUES),
    manager(&executor, &glenv, &collisionspace)
{
    allocator_Player.provider().attach(&globalstate.container_Player);
    allocator_Lifetime.provider().attach(&globalstate.container_Lifetime);
    allocator_Pickup.provider().attach(&globalstate.container_Pickup);
    allocator_RepeatSpawn.provider().attach(&globalstate.container_RepeatSpawn);
}

void Entity_LightBall_initializer(Entity* e) {
    ES_RepeatSpawn* repeatspawn = globalstate.container_RepeatSpawn.getInstance(e->entityscripts()[1]);
    repeatspawn->entity_name = "Entity_LightParticle";
    repeatspawn->lifetime = 90.0f;
    repeatspawn->spawnrate = 4.0f;
}

void Entity_LightParticle_initializer(Entity* e) {
    ES_Lifetime* particle = globalstate.container_Lifetime.getInstance(e->entityscripts()[0]);
    particle->lifetime = 24.0f;
    particle->vel = glm::vec3(-1.0f * (float(rand() % 5) / 10.0f), 0.0f, 0.0f);
}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Player::_execEntity() {
    // check cooldowns
    if (_hurt_cooldown > 0.0f)
        _hurt_cooldown -= 1.0f;
    if (_shoot_cooldown > 0.0f)
        _shoot_cooldown -= 1.0f;

    glm::vec3 &pos = entity().globaltransform().pos;

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

        globalstate.manager.spawnEntity("Entity_LightBall", entity().globaltransform());
        ES_Lifetime* ball = globalstate.container_Lifetime.getLastInstance();
        ball->lifetime = 90.0f;
        ball->vel = glm::vec3(1.75f, 0.0f, 0.0f);
    }

    pos += vel;

    if (false)
        entity().kill();
}
void ES_Player::_collide(Entity* other) {
    _hurt_cooldown = _hurt_cooldown_max;
}

ES_Player::ES_Player() :
    EntityScriptInterface(),
    _hurt_cooldown_max(120.0f),
    _hurt_cooldown(0.0f),
    _shoot_cooldown_max(30.0f),
    _shoot_cooldown(0.0f),
    _speed(0.75f),
    _last_input_dir(0.0f, 1.0f, 0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Lifetime::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (_target)
        entity().globaltransform().pos = _target->globaltransform().pos;
    else
        entity().globaltransform().pos += vel;
    
    if (lifetime == 0)
        entity().kill();
}
void ES_Lifetime::_receive(Entity *other, std::string message) {
    if (message == "target") {
        _target = other;
        if (_target)
            _target->attachNullableEntity(&_target);
    }
}
void ES_Lifetime::_collide(Entity *other) {
    entity().kill();
}

ES_Lifetime::ES_Lifetime() : EntityScriptInterface(), _target(nullptr), lifetime(0), vel(glm::vec3(0.0f)) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Pickup::_collide(Entity *other) {
    auto& i = globalstate.inventory;
    if (i.find(item_name) != i.end())
        i[item_name] += 1;
    entity().kill();
}

ES_Pickup::ES_Pickup() : EntityScriptInterface(), item_name("") {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_RepeatSpawn::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    if (spawnrate_cooldown > 0)
        spawnrate_cooldown--;
    
    if (spawnrate >= 0 && entity_name != "") {
        if (spawnrate_cooldown == 0) {
            spawnrate_cooldown = spawnrate;

            Transform spawn_transform = entity().globaltransform();
            spawn_transform.pos = spawn_transform.pos + glm::vec3(0.0f, (float(rand() % 50) / 10.0f) - 2.5f, 0.0f);
            globalstate.manager.spawnEntity(entity_name.c_str(), spawn_transform);
        }
    }
    
    if (lifetime == 0)
        entity().kill();
}

ES_RepeatSpawn::ES_RepeatSpawn() : EntityScriptInterface(), lifetime(0), spawnrate(-1), spawnrate_cooldown(0), entity_name("") {}