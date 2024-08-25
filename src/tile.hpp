#ifndef TILE_HPP_
#define TILE_HPP_

#include "../core/include/entity.hpp"

class Tile : public Entity {
    glm::vec3 _scale;

    void _initEntity();
    void _baseEntity();
    void _killEntity();
    virtual void _initTile();
    virtual void _baseTile();
    virtual void _killTile();
public:
    Tile(glm::vec3 scale);
};

#endif