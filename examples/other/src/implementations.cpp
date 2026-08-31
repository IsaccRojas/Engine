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
}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Player::_initEntity() {}

void ES_Player::_execEntity() {
    // check if hurt (does nothing for now)
    if (_hurt_cooldown > 0.0f) {
        _hurt_cooldown -= 1.0f;
    }

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

    pos += vel;

    if (false)
        entity().kill();
}

void ES_Player::_killEntity() {}
void ES_Player::_updateEntity() {}
void ES_Player::_receive(Entity* other, std::string message) {}

void ES_Player::_collide(Entity* other) {
    _hurt_cooldown = _hurt_cooldown_max;
}

ES_Player::ES_Player() :
    EntityScriptInterface(),
    _hurt_cooldown_max(120.0f),
    _hurt_cooldown(0.0f),
    _speed(0.5f),
    _last_input_dir(0.0f, 1.0f, 0.0f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Lifetime::_initEntity() {}
void ES_Lifetime::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (_target)
        entity().globaltransform().pos = _target->globaltransform().pos;
    else
        entity().globaltransform().pos += vel;
    
    if (lifetime <= 0)
        entity().kill();
}
void ES_Lifetime::_killEntity() {}
void ES_Lifetime::_updateEntity() {}
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

void ES_Pickup::_initEntity() {}
void ES_Pickup::_execEntity() {}
void ES_Pickup::_killEntity() {}
void ES_Pickup::_updateEntity() {}
void ES_Pickup::_receive(Entity *other, std::string message) {}

void ES_Pickup::_collide(Entity *other) {
    auto& i = globalstate.inventory;
    if (i.find(item_name) != i.end())
        i[item_name] += 1;
    entity().kill();
}

ES_Pickup::ES_Pickup() : EntityScriptInterface(), item_name("") {}