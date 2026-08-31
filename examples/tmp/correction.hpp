#ifndef CORRECTION_HPP_
#define CORRECTION_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../core/include/entity.hpp"
#include "../../core/include/glfwstate.hpp"
#include "../../core/include/glfwinput.hpp"

/*
    Assumes Tile and Collider are both squares and the same size
    Assumes Tile does not move
*/
class ES_Correction : public EntityScriptInterface {
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
public:
    ES_Correction();
    unsigned collider_index;
    float assist_speed;
};

struct TileInfo {
    int value;
    int quad_id_lower;
    int quad_id_upper;
};

struct MapInfo {
    glm::vec2 unit_pixel_dimensions;
    glm::vec2 coord_dimensions;
    glm::vec2 coord_origin;
    glm::ivec2 toCoords(glm::vec2 v);
    glm::vec2 toPixels(glm::ivec2 v);
    bool isValid(glm::vec2 v);
};

struct GlobalState {
    MapInfo mapinfo;
    std::vector<std::vector<TileInfo>> map;
};

extern GlobalState globalstate;

#endif