#ifndef BASIC_HPP_
#define BASIC_HPP_

#include "../core/include/object.hpp"

class Basic : public Object {
    glm::vec3 _scale;
    glm::vec3 _dimensions;

    void _initObject() override;
    void _baseObject() override;
    void _killObject() override;
    void _collisionObject(Box *box) override;

protected:
    virtual void _initBasic() = 0;
    virtual void _baseBasic() = 0;
    virtual void _killBasic() = 0;
    virtual void _collisionBasic(Box *box) = 0;

public:
    Basic(glm::vec3 scale, glm::vec3 dimensions);
};

#endif