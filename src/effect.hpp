#ifndef EFFECT_HPP_
#define EFFECT_HPP_

#include "../core/include/entity.hpp"

class Effect : public Entity {
    glm::vec3 _scale;
    int _lifetime;
    int _i;

    void _initEntity();
    void _baseEntity();
    void _killEntity();
    virtual void _initEffect();
    virtual void _baseEffect();
    virtual void _killEffect();
public:
    // if lifetime = -1, will not be killed via _baseEntity() call
    Effect(glm::vec3 scale, int lifetime);
};

#endif