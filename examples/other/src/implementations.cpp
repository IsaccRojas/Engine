#include "implementations.hpp"

GlobalResources::GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput) : 
    manager(entitymanager),
    executor(entityscriptexecutor),
    input(glfwinput)
{}

// --------------------------------------------------------------------------------------------------------------------------

glm::ivec2 MapInfo::toCoords(glm::vec2 v) {
    glm::vec2 coord_pixel_dimensions = coord_dimensions * unit_pixel_dimensions;
    glm::vec2 v_coords = v;

    v_coords -= coord_origin;           // shift pixel origin to coord origin
    v_coords /= coord_pixel_dimensions; // scale to ratio
    v_coords *= coord_dimensions;       // scale to coord
    v_coords = glm::floor(v_coords);    // floor

    return v_coords;
}

glm::vec2 MapInfo::toPixels(glm::ivec2 v) {
    glm::vec2 coord_pixel_dimensions = coord_dimensions * unit_pixel_dimensions;
    glm::vec2 v_pixels = v;

    v_pixels += 0.5f;                   // adjust to centers
    v_pixels /= coord_dimensions;       // scale to ratio
    v_pixels *= coord_pixel_dimensions; // scale to position
    v_pixels += coord_origin;           // shift coord origin to pixel origin

    return v_pixels;
}

bool MapInfo::isValid(glm::vec2 v) {
    return (v.x >= 0 && v.x < coord_dimensions.x && v.y >= 0 && v.y < coord_dimensions.y);
}

// --------------------------------------------------------------------------------------------------------------------------

void SpellInterface::_init() { _initSpell(); }
void SpellInterface::_exec() { _execSpell(); }
void SpellInterface::_kill() { _killSpell(); }
void SpellInterface::_update() { _updateSpell(); }
SpellInterface::SpellInterface() : ScriptInterface(), Resource(), pos(glm::vec3(0.0f)), dir(glm::vec3(0.0f)) {}

// --------------------------------------------------------------------------------------------------------------------------

void ES_Correction::_initEntity() {}
void ES_Correction::_execEntity() {
    MapInfo &mi = resource()->mapinfo;

    glm::vec3 base_nontile_pos = entity().globaltransform().pos;
    glm::vec3 base_nontile_prev_pos = entity().getPrevGlobalTransform().pos;
    glm::vec3 base_nontile_vel = base_nontile_pos - base_nontile_prev_pos;
    glm::vec2 base_nontile_dir = toVec2(glm::length(base_nontile_vel) == 0 ? glm::vec3(0.0f) : glm::normalize(base_nontile_vel));
    glm::vec2 nontile_coord = mi.toCoords(toVec2(base_nontile_pos));

    // check tiles in 3x3 space centered on nontile; do sides first, then corners
    glm::vec2 tile_pos;
    glm::vec2 tile_in_dir_coord;
    glm::vec2 tile_in_dir_pos;
    glm::vec3 final_nontile_pos = base_nontile_pos;
    bool can_assist;
    std::vector<glm::vec2> tile_positions{{0, 1}, {-1, 0}, {1, 0}, {0, -1}, {-1, 1}, {1, 1}, {-1, -1}, {1, -1}};
    for (auto &relative_tile_pos : tile_positions) {
        tile_pos = nontile_coord + relative_tile_pos;
        tile_in_dir_coord = nontile_coord + base_nontile_dir;
        tile_in_dir_pos = mi.toPixels(tile_in_dir_coord);

        // check if tile in direction of travel is solid; only used if travel is only along one axis
        can_assist = (mi.isValid(tile_in_dir_coord) && resource()->map[tile_in_dir_coord.x][tile_in_dir_coord.y].value <= 0);
        
        // skip if tile position is outside of map range
        if (!mi.isValid(tile_pos))
            continue;

        // check if tile value should cause correction
        if (resource()->map[tile_pos.x][tile_pos.y].value <= 0)
            continue;
        
        // detect collision
        tile_pos = mi.toPixels(tile_pos);
        if (!computeCollisionAABB(
            Transform{final_nontile_pos, toVec3(mi.unit_pixel_dimensions, 1.0f)},
            Transform{toVec3(tile_pos, 0.0f), toVec3(mi.unit_pixel_dimensions, 1.0f)}
        ))
            continue;

        // determine direction of collision and correction
        glm::vec2 dist = toVec2(final_nontile_pos) - tile_pos;
        if (glm::abs(dist.x) >= glm::abs(dist.y)) {
            // horizontal collision
            if (final_nontile_pos.x >= tile_pos.x) {
                // nt is to the right
                final_nontile_pos.x += (tile_pos.x + mi.unit_pixel_dimensions.x) - final_nontile_pos.x;

                // check if moving straight left
                if (can_assist && base_nontile_dir == glm::vec2(-1.0f, 0.0f)) {
                    // apply lesser of difference of position and +/- 1.0f in terms of absolute value
                    float diff = tile_in_dir_pos.y - base_nontile_pos.y;
                    final_nontile_pos.y += absMin(diff, assist_speed * (diff / abs(diff)));
                }
            } else {
                // nt is to the left
                final_nontile_pos.x -= final_nontile_pos.x - (tile_pos.x - mi.unit_pixel_dimensions.x);

                // check if moving straight right
                if (can_assist && base_nontile_dir == glm::vec2(1.0f, 0.0f)) {
                    // apply lesser of difference of position and +/- 1.0f in terms of absolute value
                    float diff = tile_in_dir_pos.y - base_nontile_pos.y;
                    final_nontile_pos.y += absMin(diff, assist_speed * (diff / abs(diff)));
                }
            }
        } else {
            // vertical collision
            if (final_nontile_pos.y >= tile_pos.y) {
                // nt is above
                final_nontile_pos.y += (tile_pos.y + mi.unit_pixel_dimensions.y) - final_nontile_pos.y;

                // check if moving straight down
                if (can_assist && base_nontile_dir == glm::vec2(0.0f, -1.0f)) {
                    // apply lesser of difference of position and +/- 1.0f in terms of absolute value
                    float diff = tile_in_dir_pos.x - base_nontile_pos.x;
                    final_nontile_pos.x += absMin(diff, assist_speed * (diff / abs(diff)));
                }
            } else {
                // nt is below
                final_nontile_pos.y -= final_nontile_pos.y - (tile_pos.y - mi.unit_pixel_dimensions.y);

                // check if moving straight up
                if (can_assist && base_nontile_dir == glm::vec2(0.0f, 1.0f)) {
                    // apply lesser of difference of position and +/- 1.0f in terms of absolute value
                    float diff = tile_in_dir_pos.x - base_nontile_pos.x;
                    final_nontile_pos.x += absMin(diff, assist_speed * (diff / abs(diff)));
                }
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
ES_Correction::ES_Correction() : EntityScriptInterface(), Resource(), collider_index(0), assist_speed(0.5f) {}

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

    // get velocity as sum of input directions
    glm::vec3 vel = glm::vec3(
        float(-1.0f * resource()->input->get_a()) + float(resource()->input->get_d()),
        float(-1.0f * resource()->input->get_s()) + float(resource()->input->get_w()),
        0.0f
    );
    
    // normalize velocity length to speed
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