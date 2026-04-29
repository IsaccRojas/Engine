#include "implementations.hpp"

GlobalResources::GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput) : 
    manager(entitymanager),
    executor(entityscriptexecutor),
    input(glfwinput)
{}

// --------------------------------------------------------------------------------------------------------------------------

void SpellInterface::_init() { _initSpell(); }
void SpellInterface::_exec() { _execSpell(); }
void SpellInterface::_kill() { _killSpell(); }
void SpellInterface::_update() { _updateSpell(); }
SpellInterface::SpellInterface() : ScriptInterface(), Resource(), pos(glm::vec3(0.0f)), dir(glm::vec3(0.0f)) {}

// --------------------------------------------------------------------------------------------------------------------------

void Spell_LightBallSpell::_initSpell() {}

void Spell_LightBallSpell::_execSpell() {
    // spawn light ball
    resource()->manager->spawnEntity("Entity_LightBall", Transform{pos, glm::vec3(1.0f)});
    ES_Lifetime* lightball_lifetime = resource()->container_Lifetime.getLastInstance();
    lightball_lifetime->lifetime = 90;
    lightball_lifetime->vel = dir;
    
    enqueueKill();
}

void Spell_LightBallSpell::_killSpell() {}
void Spell_LightBallSpell::_updateSpell() {}

Spell_LightBallSpell::Spell_LightBallSpell() : SpellInterface() {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Player::_initEntity() {
    _castables.push_back(Castable{
        nullptr,
        CASTTYPE_STAVE,
        "Spell_LightBallSpell",
        30,
        glm::vec3(0.0f),
        glm::vec3(0.0f, -0.5f, 0.0f)
    });
}

void ES_Player::_execEntity() {
    // check if hurt (does nothing for now)
    if (_hurt_cooldown > 0.0f) {
        _hurt_cooldown -= 1.0f;
    }

    glm::vec3 &pos = entity().globaltransform().pos;

    glm::vec3 vel = glm::vec3(
        float(-1.0f * resource()->input->get_a()) + float(resource()->input->get_d()),
        float(-1.0f * resource()->input->get_s()) + float(resource()->input->get_w()),
        0.0f
    );
    if (glm::length(vel))
        vel = _speed * glm::normalize(vel);

    pos += vel;
    
    if (_cast_cooldown <= 0.0f && resource()->input->get_m1()) {
        // select first cast for now
        Castable &castable = *(_castables.begin());
        
        // check if already casted
        bool cast_found = false;
        for (auto &c : _casts) {
            if (&castable == c.source) {
                cast_found = true;
                break;
            }
        }
        
        // cast
        if (!cast_found) {
            _casts.push_back(castable);
            _casts.back().source = &castable;
            _cast_cooldown = _cast_cooldown_max;
        }
    }
    if (_cast_cooldown > 0.0f)
        _cast_cooldown -= 1.0f;
    
    checkCasts();

    if (resource()->input->get_space())
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
    Resource(),
    _hurt_cooldown_max(120.0f),
    _hurt_cooldown(0.0f),
    _cast_cooldown_max(24.0f),
    _cast_cooldown(0.0f),
    _speed(0.5f)
{}

void ES_Player::checkCasts() {
    auto iter = _casts.begin();
    while (iter != _casts.end()) {
        Castable& cast = *iter;

        if (cast.cast_time <= 0) {
            // spawn spell
            resource()->executor->spawnScript(cast.spell_entity_name.c_str());
            SpellInterface* spell = resource()->container_Spells.getLastInstance();
            spell->pos = (cast.type == CASTTYPE_TOME) ? cast.pos : entity().globaltransform().pos;
            spell->dir = cast.dir;
            
            iter = _casts.erase(iter);
            continue;
        }

        // advance cast time
        cast.cast_time--;
        iter++;
    }
}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Chaser::_initEntity() {}

void ES_Chaser::_execEntity() {
    // find target if one is not stored
    if (!_target)
        // iterate on all players
        for (
            auto group_player_iter = resource()->manager->groupBegin("Group_Player");
            group_player_iter != resource()->manager->groupEnd("Group_Player");
            ++group_player_iter
        ) {
            // store and lockout player if it is not kill enqueued
            if (!((*group_player_iter)->entityscripts()[0]->getKillEnqueued())) {
                _target = (*group_player_iter);
                _target->entityscripts()[0]->lockout(&this->key());
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
        if (_target->entityscripts()[0]->getKillEnqueued()) {
            _target->entityscripts()[0]->unlock(&this->key());
            _target = nullptr;
        }
    }
}

void ES_Chaser::_killEntity() {
    // unlock target in case it is stored
    if (_target)
        _target->entityscripts()[0]->unlock(&this->key());
}

void ES_Chaser::_updateEntity() {}
void ES_Chaser::_receive(Entity *other, std::string message) {}
void ES_Chaser::_collide(Entity *other) {}

ES_Chaser::ES_Chaser() : EntityScriptInterface(), Resource(), _target(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Lifetime::_initEntity() {}

void ES_Lifetime::_execEntity() {
    if (lifetime > 0)
        lifetime--;
    
    if (_target) {
        entity().globaltransform().pos = _target->globaltransform().pos;

        if (_target->entityscripts()[0]->getKillEnqueued()) {
            _target->entityscripts()[0]->unlock(&key());
            _target = nullptr;
        }
    } else
        entity().globaltransform().pos += vel;

    if (lifetime <= 0)
        entity().kill();
}

void ES_Lifetime::_killEntity() {
    if (_target)
        _target->entityscripts()[0]->unlock(&key());
}

void ES_Lifetime::_updateEntity() {}

void ES_Lifetime::_receive(Entity *other, std::string message) {
    if (message == "target") {
        _target = other;
        _target->entityscripts()[0]->lockout(&key());
    }
}
void ES_Lifetime::_collide(Entity *other) {}

ES_Lifetime::ES_Lifetime() : EntityScriptInterface(), _target(nullptr), lifetime(0), vel(glm::vec3(0.0f)) {}