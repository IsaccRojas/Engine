#ifndef BASIC_HPP_
#define BASIC_HPP_

#include "../core/include/object.hpp"

class Basic : public Object {
    glm::vec3 _scale;

    void _initObject();
    void _baseObject();
    void _killObject();
    void _collisionObject(Box *box);
    virtual void _initBasic();
    virtual void _baseBasic();
    virtual void _killBasic();
    virtual void _collisionBasic(Box *box);
public:
    Basic(glm::vec3 scale);
};

#endif