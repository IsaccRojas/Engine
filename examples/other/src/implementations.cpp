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

void ES_Correction::_initEntity() {}
void ES_Correction::_execEntity() {
    /*
        assume Tile and Collider are both squares and the same size
        assume Tile does not move
    */

    glm::vec2 pixel_dimensions(resource()->pixel_width, resource()->pixel_height);
    glm::vec2 tile_dimensions(resource()->tile_rows, resource()->tile_columns);
    glm::vec2 unit_dimensions(resource()->unit_pixel_width, resource()->unit_pixel_height);
    glm::vec3 base_nontile_pos = entity().entitycolliders()[collider_index]->getCurrentTransformation().pos;

    // transform pixel position into tile coordinate
    glm::vec2 nontile_coord = to_vec2(base_nontile_pos);
    nontile_coord.y *= -1.0f;                   // invert y
    nontile_coord += pixel_dimensions / 2.0f;   // shift pixel origin to tile origin
    nontile_coord /= pixel_dimensions;          // scale to ratio
    nontile_coord *= tile_dimensions;           // scale to coord
    nontile_coord = glm::floor(nontile_coord);  // floor

    // TODO: add in like this to player entity, to test it
    std::cout << "(" << nontile_coord.x << ", " << nontile_coord.y << ")" << std::endl;

    // check tiles in 3x3 space centered on nontile
    glm::vec2 tile_pos;
    glm::vec3 final_nontile_pos = base_nontile_pos;
    for (int x = nontile_coord.x - 1; x <= nontile_coord.x + 1; x++) {
        if (x < 0 || x >= tile_dimensions.x)
            continue;
        for (int y = nontile_coord.y - 1; y <= nontile_coord.y + 1; y++) {
            if (y < 0 || y >= tile_dimensions.y)
                continue;
            
            if (resource()->map[y][x].value <= 0)
                continue;
            
            // transform tile coordinate into pixel position
            tile_pos = glm::vec2(x, y);             // already unfloored
            tile_pos /= tile_dimensions;            // scale to ratio
            tile_pos *= pixel_dimensions;           // scale to position
            tile_pos -= pixel_dimensions / 2.0f;    // shift tile origin to pixel origin
            tile_pos.y *= -1.0f;                    // invert y
            
            // detect collision
            if (!computeCollisionAABB(
                Transform{final_nontile_pos, to_vec3(unit_dimensions, 1.0f)},
                Transform{to_vec3(tile_pos, 0.0f), to_vec3(unit_dimensions, 1.0f)}
            ))
                continue;

            // determine direction of collision and correction
            glm::vec2 dist = to_vec2(final_nontile_pos) - tile_pos;
            if (glm::abs(dist.x) >= glm::abs(dist.y)) {
                // horizontal collision
                if (final_nontile_pos.x >= tile_pos.x)
                    // nt is to the right
                    final_nontile_pos.x += (tile_pos.x + unit_dimensions.x) - final_nontile_pos.x;
                else
                    // nt is to the left
                    final_nontile_pos.x -= (tile_pos.x + unit_dimensions.x) - final_nontile_pos.x;
            } else {
                // vertical collision
                if (final_nontile_pos.y >= tile_pos.y)
                    // nt is above
                    final_nontile_pos.y += (tile_pos.y + unit_dimensions.y) - final_nontile_pos.y;
                else
                    // nt is below
                    final_nontile_pos.y -= (tile_pos.y + unit_dimensions.y) - final_nontile_pos.y;
            }
        }
    }

    // apply final shift
    entity().globaltransform().pos += final_nontile_pos - base_nontile_pos;
}
void ES_Correction::_killEntity() {}
void ES_Correction::_updateEntity() {}
void ES_Correction::_receive(Entity *other, std::string message) {}
void ES_Correction::_collide(Entity *other) {}
ES_Correction::ES_Correction() : EntityScriptInterface(), Resource(), collider_index(0) {}

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