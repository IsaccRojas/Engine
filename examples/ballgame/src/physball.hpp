#ifndef PHYSBALL_HPP_
#define PHYSBALL_HPP_

#include "../../../core/include/entity.hpp"

class PhysBall : public Entity {
    Quad *_quad;
    Box *_box;
    
    std::string _animation_name;
    std::string _filter_name;

    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;

protected:
    virtual void _initPhysBall();
    virtual void _basePhysBall();
    virtual void _killPhysBall();

public:
    PhysBall(std::string animation_name, std::string filter_name);
    Quad *quad();
    Box *box();
};

#endif