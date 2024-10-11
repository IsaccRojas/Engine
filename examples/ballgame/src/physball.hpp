#ifndef PHYSBALL_HPP_
#define PHYSBALL_HPP_

#include "../../../core/include/entity.hpp"

/* Representation of a sphere in physical space. */
class ValueSphere : public ColliderInterface<ValueSphere> {
public:
    ValueSphere() : radius(0.0f), value(0.0f) {}
    ValueSphere(ValueSphere &&other) { operator=(std::move(other)); }
    ValueSphere(const ValueSphere &other) = delete;
    ~ValueSphere() { /* automatic destruction is fine */ }

    ValueSphere &operator=(ValueSphere &&other) {
        if (this != &other) {
            ColliderInterface<ValueSphere>::operator=(std::move(other));
            radius = other.radius;
            other.radius = 0.0f;
        }
        return *this;
    }
    ValueSphere &operator=(const ValueSphere &other) = delete;

    float radius;
    float value;

    /* Ignores scale. */
    bool computeCollision(ValueSphere *other) override {
        return ((glm::distance(transform.pos, other->transform.pos) - (radius + other->radius)) < 0.0f);
    }
};

class PhysBall : public Entity {
    Quad *_quad;
    unsigned _quad_off;
    Sphere *_sphere;
    
    std::string _animation_name;
    std::string _filter_name;

    void _initEntity() override;
    void _baseEntity() override;
    void _killEntity() override;

protected:
    virtual void _initPhysBall();
    virtual void _basePhysBall();
    virtual void _killPhysBall();
    virtual void _onCollision(Sphere *other);

public:
    PhysBall(std::string animation_name, std::string filter_name);
    Quad *quad();
    Sphere *sphere();
    glm::vec3 vel;
    float health;
};

#endif