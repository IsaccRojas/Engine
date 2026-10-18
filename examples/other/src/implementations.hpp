#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalState;

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightBall : public EntityInterface {
    void _initEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightParticle : public EntityInterface {
    void _initEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Player : public EntityInterface {
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _shoot_cooldown_max;
    float _shoot_cooldown;
    float _speed;
    glm::vec3 _last_input_dir;
    void _execEntity() override;
    void _collideEntity(EntityInterface* other) override;
public:
    Entity_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

class BeingInterface : public EntityInterface {
    float hurt_cooldown;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _collideEntity(EntityInterface* other) override;

    virtual void _initBeing();
    virtual void _execBeing();
    virtual void _killBeing();
    virtual void _collideBeing(EntityInterface* other);
public:
    BeingInterface();
    float hurt_cooldown_max;
    float health;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class Entity_Lifetime : public EntityInterface {
    EntityInterface* _target;
    void _execEntity() override;
    void _collideEntity(EntityInterface* other) override;
public:
    Entity_Lifetime();
    void setTarget(EntityInterface* target);
    int lifetime;
    glm::vec3 vel;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Pickup
    Upon collision, increments the name on the GlobalResources' inventory map and kills itself.

    std::string item_name - name to increment in GlobalResources' inventory
*/
class Entity_Pickup : public EntityInterface {
    void _collideEntity(EntityInterface* other) override;
public:
    Entity_Pickup();
    std::string item_name;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity  RepeatSpawn
    Repeatedly spawns an entity at its position at the specified rate, and kills itself after the specified
    lifetime. Does not kill itself if lifetime is negative. Spawns nothing if spawnrate is 0 or entity name is "".
*/
class Entity_RepeatSpawn : public EntityInterface {
    void _execEntity() override;
public:
    Entity_RepeatSpawn();
    int lifetime;
    int spawnrate;
    int spawnrate_cooldown;
    std::string entity_name;
};

// --------------------------------------------------------------------------------------------------------------------------

struct GlobalState {
    GlobalState();

    GLFWState glfwstate;
    GLFWInput input;

    std::unordered_map<std::string, Animation> animations;
    std::unordered_map<std::string, Filter> filters;

    GLEnv glenv;
    CollisionSpace collisionspace;
    EntityExecutor executor;

    GenericProvidingEntityAllocator<Entity_LightBall> allocator_LightBall;
    GenericProvidingEntityAllocator<Entity_LightParticle> allocator_LightParticle;
    GenericProvidingEntityAllocator<Entity_Player> allocator_Player;
    GenericProvidingEntityAllocator<Entity_Pickup> allocator_Pickup;
    GenericProvidingEntityAllocator<Entity_Lifetime> allocator_Lifetime;
    GenericProvidingEntityAllocator<Entity_RepeatSpawn> allocator_RepeatSpawn;
    
    RefContainer<Entity_Player> container_Player;
    RefContainer<Entity_Lifetime> container_Lifetime;
    RefContainer<Entity_Pickup> container_Pickup;
    RefContainer<Entity_RepeatSpawn> container_RepeatSpawn;

    EntityNodeContainer<Entity_Lifetime> nodecontainer_Lifetime;

    std::unordered_map<std::string, int> inventory;
};

extern GlobalState globalstate;

#endif