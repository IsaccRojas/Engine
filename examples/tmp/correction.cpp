#include "correction.hpp"

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

void ES_Correction::_initEntity() {}
void ES_Correction::_execEntity() {
    MapInfo &mi = globalstate.mapinfo;

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
        can_assist = (mi.isValid(tile_in_dir_coord) && globalstate.map[tile_in_dir_coord.x][tile_in_dir_coord.y].value <= 0);
        
        // skip if tile position is outside of map range
        if (!mi.isValid(tile_pos))
            continue;

        // check if tile value should cause correction
        if (globalstate.map[tile_pos.x][tile_pos.y].value <= 0)
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
ES_Correction::ES_Correction() : EntityScriptInterface(), collider_index(0), assist_speed(0.5f) {}