#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "../../../core/include/entity.hpp"

class Object : public Entity {
    Quad *_quad;
    unsigned _quad_off;
    Box *_box;
    
    std::string _animation_name;
    std::string _filter_name;

    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;

protected:
    virtual void _initObject();
    virtual void _baseObject();
    virtual void _killObject();
    virtual void _onCollision(Box *other);

public:
    Object(std::string animation_name, std::string filter_name);
    Quad *quad();
    Box *box();
    glm::vec3 vel;
};

#endif