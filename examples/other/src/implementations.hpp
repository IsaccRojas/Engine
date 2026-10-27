#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalState;
class Entity_Travel;
// --------------------------------------------------------------------------------------------------------------------------

class Entity_Enemy_Chaser : public EntityInterface {
    Entity_Travel* _entity_travel;
    void _initEntity() override;
    void _execEntity() override;
    void _collideEntity(EntityInterface* other) override;
public:
    Entity_Enemy_Chaser();
    float speed;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightBall : public EntityInterface {
    void _initEntity() override;
    void _collideEntity(EntityInterface* other) override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightParticle : public EntityInterface {
    void _initEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Player : public EntityInterface {
    float _shoot_cooldown_max;
    float _shoot_cooldown;
    float _speed;
    glm::vec3 _last_input_dir;
    void _initEntity() override;
    void _execEntity() override;
public:
    Entity_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Being : public EntityInterface {
    float hurt_cooldown;
    void _execEntity() override;
    void _collideEntity(EntityInterface* other) override;
public:
    Entity_Being();
    float hurt_cooldown_max;
    float health;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Hitbox : public EntityInterface {
public:
    Entity_Hitbox();
    float damage;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity_Travel
    Applies specified velocity to global transform. Applies difference in position between itself and target, normalized and
    scaled with specified velocity's magnitude, if target is not nullptr.
*/
class Entity_Travel : public EntityInterface {
    void _execEntity() override;
public:
    Entity_Travel();
    glm::vec3 vel;
    EntityInterface* target;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class Entity_Lifetime : public EntityInterface {
    void _execEntity() override;
public:
    Entity_Lifetime();
    int lifetime;
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

    Resource<Entity_Enemy_Chaser> resource_Enemy_Chaser;
    Resource<Entity_LightBall> resource_LightBall;
    Resource<Entity_LightParticle> resource_LightParticle;
    Resource<Entity_Player> resource_Player;
    Resource<Entity_Being> resource_Being;
    Resource<Entity_Hitbox> resource_Hitbox;
    Resource<Entity_Travel> resource_Travel;
    Resource<Entity_Lifetime> resource_Lifetime;
    Resource<Entity_Pickup> resource_Pickup;
    Resource<Entity_RepeatSpawn> resource_RepeatSpawn;

    std::unordered_map<std::string, int> inventory;
};

extern GlobalState globalstate;

#endif