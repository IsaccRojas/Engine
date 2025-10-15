#ifndef EFFECT_HPP_
#define EFFECT_HPP_

#include "../../../core/include/entity.hpp"

class Effect : public Entity {
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
    
    virtual void _initEffect();
    virtual void _baseEffect();
    virtual void _killEffect();

public:
    /* If lifetime is negative, this effect does not kill itself. */
    Effect(std::string animation_name, int lifetime, DrawType type);
    Quad *quad();
};

#endif