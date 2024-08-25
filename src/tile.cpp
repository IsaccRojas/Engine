#include "tile.hpp"

void Tile::_initEntity() {
    _initTile();

    getQuad()->scale.v = _scale;
    getQuad()->pos.v.z = -1.0f;
}

void Tile::_baseEntity() {
    _baseTile();

    stepAnim();
    enqueue();
}

void Tile::_killEntity() {
    _killTile();

    removeQuad();
}

void Tile::_initTile() {}
void Tile::_baseTile() {}
void Tile::_killTile() {}

Tile::Tile(glm::vec3 scale) : Entity(), _scale(scale) {}