#include "implementations.hpp"

GlobalResources::GlobalResources(GLFWInput* glfw_input) : input(glfw_input) {}

// --------------------------------------------------------------------------------------------------------------------------

ResourcesMixin::ResourcesMixin(GlobalResources* resources) : _resources(resources) {}

GlobalResources& ResourcesMixin::resources() { return *_resources; }

// --------------------------------------------------------------------------------------------------------------------------

void SpellInterface::_init() {}
void SpellInterface::_exec() {
    // new execution lifetime, set _executing and call _startSpell()
    if (!_executing) {
        _executing = true;
        _startSpell();
    }

    _execSpell();

    // if _end_execution not set, keep enqueuing
    if (!_end_execution)
        enqueueExec(_execution_queue);
    // else, unset _executing
    else {
        _end_execution = false;
        _executing = false;
    }
}
void SpellInterface::_kill() {}
void SpellInterface::_update() {}
SpellInterface::SpellInterface(unsigned execution_queue, GlobalResources* resources) :
    ScriptInterface(),
    ResourcesMixin(resources),
    _execution_queue(execution_queue),
    _executing(false),
    _end_execution(false)
{};

void SpellInterface::endSpell() {
    _end_execution = true;
}

// --------------------------------------------------------------------------------------------------------------------------

void Spell_LightBall::_startSpell() {}
void Spell_LightBall::_execSpell() {}
Spell_LightBall::Spell_LightBall(unsigned execution_queue, GlobalResources* resources) : SpellInterface(execution_queue, resources) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Player::_initEntity() {}

void ES_Player::_execEntity() {
    // check if hurt (does nothing for now)
    if (_hurt_cooldown > 0.0f) {
        _hurt_cooldown -= 1.0f;
    }

    glm::vec3 &pos = entity().globaltransform().pos;

    glm::vec3 vel = glm::vec3(
        float(-1.0f * resources().input->get_a()) + float(resources().input->get_d()),
        float(-1.0f * resources().input->get_s()) + float(resources().input->get_w()),
        0.0f
    );
    if (glm::length(vel))
        vel = _speed * glm::normalize(vel);

    pos += vel;
    
    if (_hitbox_cooldown <= 0.0f && resources().input->get_m1()) {
        // spawn hitbox
        Entity* hitbox = entity().manager().spawnEntity("Entity_Hitbox", Transform{pos, glm::vec3(24.0f, 24.0f, 24.0f)});
        resources().provider_ES_Lifetime.getInstance(hitbox)->lifetime = 22;

        // spawn slash effect and set self as its target
        Entity* slash = entity().manager().spawnEntity("Entity_Slash", Transform{pos, glm::vec3(1.0f)});
        ES_Lifetime* slash_lifetime = resources().provider_ES_Lifetime.getInstance(slash);
        slash_lifetime->receive(&entity(), "target");
        slash_lifetime->lifetime = 18;

        _hitbox_cooldown = _hitbox_cooldown_max;
    }
    if (_hitbox_cooldown > 0.0f)
        _hitbox_cooldown -= 1.0f;
    
    if (resources().input->get_space())
        enqueueKill();
}

void ES_Player::_killEntity() {}
void ES_Player::_updateEntity() {}
void ES_Player::_receive(Entity* other, std::string message) {}

void ES_Player::_collide(Entity* other) {
    _hurt_cooldown = _hurt_cooldown_max;
}

ES_Player::ES_Player(GlobalResources* resources) :
    EntityScriptInterface(),
    ResourcesMixin(resources),
    _hurt_cooldown_max(120.0f), 
    _hurt_cooldown(0.0f), 
    _hitbox_cooldown_max(24.0f),
    _hitbox_cooldown(0.0f),
    _speed(0.5f)
{}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Chaser::_initEntity() {}

void ES_Chaser::_execEntity() {
    // find target if one is not stored
    if (!_target)
        // iterate on all players
        for (
            auto group_player_iter = entity().manager().groupBegin("Group_Player");
            group_player_iter != entity().manager().groupEnd("Group_Player");
            ++group_player_iter
        ) {
            // store and lockout player if it is not kill enqueued
            if (!((*group_player_iter)->entityscriptview().getKillEnqueued())) {
                _target = (*group_player_iter);
                _target->entityscriptview().lockout(&this->key());
                break;
            }
        }
    
    glm::vec3 &pos = entity().globaltransform().pos;

    // chase target if one is stored
    if (_target) {
        float speed = 0.25f;
        glm::vec3 dir = _target->globaltransform().pos - pos;
        if (glm::length(dir))
            pos += speed * glm::normalize(dir);
        
        // lose reference and unlock player if it is kill enqueued
        if (_target->entityscriptview().getKillEnqueued()) {
            _target->entityscriptview().unlock(&this->key());
            _target = nullptr;
        }
    }
}

void ES_Chaser::_killEntity() {
    // unlock target in case it is stored
    if (_target)
        _target->entityscriptview().unlock(&this->key());
}

void ES_Chaser::_updateEntity() {}
void ES_Chaser::_receive(Entity *other, std::string message) {}
void ES_Chaser::_collide(Entity *other) {}

ES_Chaser::ES_Chaser() : EntityScriptInterface(), _target(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Lifetime::_initEntity() {}

void ES_Lifetime::_execEntity() {    
    if (lifetime > 0)
        lifetime--;
    
    if (_target) {
        entity().globaltransform().pos = _target->globaltransform().pos;

        if (_target->entityscriptview().getKillEnqueued()) {
            _target->entityscriptview().unlock(&key());
            _target = nullptr;
        }
    } else
        entity().globaltransform().pos += vel;

    if (lifetime <= 0)
        enqueueKill();
}

void ES_Lifetime::_killEntity() {
    if (_target)
        _target->entityscriptview().unlock(&key());
}

void ES_Lifetime::_updateEntity() {}

void ES_Lifetime::_receive(Entity *other, std::string message) {
    if (message == "target") {
        _target = other;
        _target->entityscriptview().lockout(&key());
    }
}
void ES_Lifetime::_collide(Entity *other) {}

ES_Lifetime::ES_Lifetime() : EntityScriptInterface(), _target(nullptr), lifetime(0), vel(glm::vec3(0.0f)) {}