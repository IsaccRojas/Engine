#ifndef GFXENTITY_HPP_
#define GFXENTITY_HPP_

#include "../../../core/include/entity.hpp"

class GfxEntity : public Entity {
    DrawType _quad_type;
    Quad *_quad;
    unsigned _quad_off;
    std::string _animation_name;

    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;

protected:
    int _lifetime;
    int _i;
    
    virtual void _initGfxEntity();
    virtual void _baseGfxEntity();
    virtual void _killGfxEntity();

public:
    /* If lifetime is negative, this effect does not kill itself. */
    GfxEntity(std::string animation_name, int lifetime, DrawType type);
    Quad *quad();
};

#endif