#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalState;

// --------------------------------------------------------------------------------------------------------------------------

class ES_LightBall_Master : public EntityInterface {
    void _initEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class ES_LightParticle_Master : public EntityInterface {
    void _initEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class ES_Player : public EntityInterface {
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _shoot_cooldown_max;
    float _shoot_cooldown;
    float _speed;
    glm::vec3 _last_input_dir;
    void _execEntity() override;
    void _collide(EntityInterface* other) override;
public:
    ES_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

class ES_Being : public EntityInterface {
    float hurt_cooldown;
    void _execEntity() override;
    void _collide(EntityInterface* other) override;
public:
    ES_Being();
    float health;
    float hurt_cooldown_max;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class ES_Lifetime : public EntityInterface {
    EntityInterface* _target;
    void _execEntity() override;
    void _receive(EntityInterface* other, std::string message) override;
    void _collide(EntityInterface* other) override;
public:
    ES_Lifetime();
    int lifetime;
    glm::vec3 vel;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Pickup
    Upon collision, increments the name on the GlobalResources' inventory map and kills itself.

    std::string item_name - name to increment in GlobalResources' inventory
*/
class ES_Pickup : public EntityInterface {
    void _collide(EntityInterface* other) override;
public:
    ES_Pickup();
    std::string item_name;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity  RepeatSpawn
    Repeatedly spawns an entity at its position at the specified rate, and kills itself after the specified
    lifetime. Does not kill itself if lifetime is negative. Spawns nothing if spawnrate is 0 or entity name is "".
*/
class ES_RepeatSpawn : public EntityInterface {
    void _execEntity() override;
public:
    ES_RepeatSpawn();
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

    GenericProvidingEntityAllocator<ES_LightBall_Master> allocator_LightBall_Master;
    GenericProvidingEntityAllocator<ES_LightParticle_Master> allocator_LightParticle_Master;
    GenericProvidingEntityAllocator<ES_Player> allocator_Player;
    GenericProvidingEntityAllocator<ES_Pickup> allocator_Pickup;
    GenericProvidingEntityAllocator<ES_Lifetime> allocator_Lifetime;
    GenericProvidingEntityAllocator<ES_RepeatSpawn> allocator_RepeatSpawn;
    
    RefContainer<ES_Player> container_Player;
    RefContainer<ES_Lifetime> container_Lifetime;
    RefContainer<ES_Pickup> container_Pickup;
    RefContainer<ES_RepeatSpawn> container_RepeatSpawn;

    std::unordered_map<std::string, int> inventory;
};

extern GlobalState globalstate;

#endif