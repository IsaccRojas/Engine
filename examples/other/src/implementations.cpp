#include "implementations.hpp"

void ES_Player::_initEntity() {
    entity().quads()[0]->animationstate().setCycleState("normal");
}

void ES_Player::_execEntity() {
    // check if hurt and set appropriate animation state and collision state
    if (_hurt_cooldown > 0.0f) {
        _hurt_cooldown -= 1.0f;
        entity().entitycolliderviews()[0].collision_enabled() = false;
        entity().quads()[0]->animationstate().setCycleState("hurt");
    } else {
        entity().entitycolliderviews()[0].collision_enabled() = true;
        entity().quads()[0]->animationstate().setCycleState("normal");
    }

    glm::vec3 &pos = entity().globaltransform().pos;

    float speed = 0.5f;
    glm::vec3 dir = glm::vec3(
        float(-1.0f * _input_state->get_a()) + float(_input_state->get_d()),
        float(-1.0f * _input_state->get_s()) + float(_input_state->get_w()),
        0.0f
    );
    if (glm::length(dir))
        pos += speed * glm::normalize(dir);
    
    if (_hitbox_cooldown <= 0.0f && _input_state->get_m1()) {
        entity().manager().spawnEntity("Entity_Hitbox", Transform{pos, glm::vec3(24.0f, 24.0f, 24.0f)});
        _hitbox_cooldown = _hitbox_cooldown_max;
    }
    if (_hitbox_cooldown > 0.0f)
        _hitbox_cooldown -= 1.0f;
    
    if (_input_state->get_space())
        enqueueKill();
}

void ES_Player::_killEntity() {}
void ES_Player::_updateEntity() {}
void ES_Player::_receive(Entity *other, std::string message) {}

void ES_Player::_collide(Entity *other) {
    _hurt_cooldown = _hurt_cooldown_max;
}

ES_Player::ES_Player(GLFWInput *input_state) :
    EntityScript(), 
    _input_state(input_state), 
    _hurt_cooldown_max(120.0f), 
    _hurt_cooldown(0.0f), 
    _hitbox_cooldown_max(120.0f), 
    _hitbox_cooldown(0.0f)
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
void ES_Chaser::_receive(Entity *entity, std::string message) {}
void ES_Chaser::_collide(Entity *entity) {}

ES_Chaser::ES_Chaser() : EntityScript(), _target(nullptr) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Hitbox::_initEntity() {}
void ES_Hitbox::_execEntity() {
    _lifetime--;
    if (_lifetime <= 0)
        enqueueKill();
}
void ES_Hitbox::_killEntity() {}
void ES_Hitbox::_updateEntity() {}
void ES_Hitbox::_receive(Entity *entity, std::string message) {}
void ES_Hitbox::_collide(Entity *entity) {
    std::cout << "hitbox collision" << std::endl;
}

ES_Hitbox::ES_Hitbox() : EntityScript(), _lifetime(120) {}