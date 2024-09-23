#ifndef GFXBALL_HPP_
#define GFXBALL_HPP_

#include "../../../core/include/entity.hpp"

class GfxBall : public Entity {
    Quad *_quad;
    unsigned _quad_off;
    std::string _animation_name;

    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;

protected:
    int _lifetime;
    int _i;
    
    virtual void _initGfxBall();
    virtual void _baseGfxBall();
    virtual void _killGfxBall();

public:
    /* If lifetime is negative, this effect does not kill itself. */
    GfxBall(std::string animation_name, int lifetime);
    Quad *quad();
};

#endif