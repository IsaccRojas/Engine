#include "implementations.hpp"

GlobalResources::GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput) : 
    manager(entitymanager),
    executor(entityscriptexecutor),
    input(glfwinput),
    provider_Player(this),
    provider_Chaser(this),
    provider_Spell_LightBall(this)
{}

// --------------------------------------------------------------------------------------------------------------------------

ResourcesMixin::ResourcesMixin(GlobalResources* resources) : _resources(resources) {}

GlobalResources& ResourcesMixin::resources() { return *_resources; }

// --------------------------------------------------------------------------------------------------------------------------

void S_Spell_LightBall::_init() {}

void S_Spell_LightBall::_exec() {
    // spawn light ball
    Entity* lightball = resources().manager->spawnEntity("Entity_LightBall", Transform{glm::vec3(0.0f), glm::vec3(1.0f)});
    ES_Lifetime* lightball_lifetime = resources().provider_Lifetime.getInstance(lightball);
    lightball_lifetime->lifetime = 90;
    lightball_lifetime->vel = glm::vec3(0.0f, -1.0f, 0.0f);
}

void S_Spell_LightBall::_kill() {}
void S_Spell_LightBall::_update() {}

S_Spell_LightBall::S_Spell_LightBall(GlobalResources* resources) : ScriptInterface(), ResourcesMixin(resources) {}

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
        Entity* hitbox = resources().manager->spawnEntity("Entity_Hitbox", Transform{pos, glm::vec3(24.0f, 24.0f, 24.0f)});
        resources().provider_Lifetime.getInstance(hitbox)->lifetime = 22;

        // spawn slash effect and set self as its target
        Entity* slash = resources().manager->spawnEntity("Entity_Slash", Transform{pos, glm::vec3(1.0f)});
        ES_Lifetime* slash_lifetime = resources().provider_Lifetime.getInstance(slash);
        slash_lifetime->receive(&entity(), "target");
        slash_lifetime->lifetime = 18;
        
        // spawn light ball
        resources().executor->spawnScript("S_Spell_LightBall", 0);

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
            auto group_player_iter = resources().manager->groupBegin("Group_Player");
            group_player_iter != resources().manager->groupEnd("Group_Player");
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

ES_Chaser::ES_Chaser(GlobalResources* resources) : EntityScriptInterface(), ResourcesMixin(resources), _target(nullptr) {}

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