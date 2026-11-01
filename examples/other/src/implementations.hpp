#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalState;
class Entity_Travel;
class Entity_AcquirePlayer;

// --------------------------------------------------------------------------------------------------------------------------

class Entity_FireParticle : public EntityInterface {
    void _onInitEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_FireBall : public EntityInterface {
    void _onInitEntity() override;
    void _onCollisionEntity(EntityInterface* other) override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Shooter : public EntityInterface {
    float _shoot_cooldown_max;
    float _shoot_cooldown;
    Entity_Travel* t;
    Entity_AcquirePlayer* aq;
    void _onInitEntity() override;
    void _onExecEntity() override;
    void _onKillEnqueueEntity() override;
    void _onCollisionEntity(EntityInterface* other) override;
public:
    Entity_Shooter();
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Chaser : public EntityInterface {
    Entity_Travel* t;
    Entity_AcquirePlayer* aq;
    void _onInitEntity() override;
    void _onExecEntity() override;
    void _onKillEnqueueEntity() override;
    void _onCollisionEntity(EntityInterface* other) override;
public:
    Entity_Chaser();
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightBall : public EntityInterface {
    void _onInitEntity() override;
    void _onCollisionEntity(EntityInterface* other) override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_LightParticle : public EntityInterface {
    void _onInitEntity() override;
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Player : public EntityInterface {
    float _shoot_cooldown_max;
    float _shoot_cooldown;
    float _speed;
    glm::vec3 _last_input_dir;
    void _onInitEntity() override;
    void _onExecEntity() override;
public:
    Entity_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

class Entity_Being : public EntityInterface {
    float hurt_cooldown;
    void _onExecEntity() override;
    void _onCollisionEntity(EntityInterface* other) override;
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
    Entity_AcquirePlayer
    Manages acquiring a random player. player is nullptr if none available.
*/
class Entity_AcquirePlayer : public EntityInterface {
    void _onExecEntity() override;
public:
    Entity_AcquirePlayer();
    Entity_Player* player;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity_Travel
    Applies specified velocity to global transform. 
    If target is true, applies difference in position between itself and pos normalized by accel_speed, no greater 
    than max_speed.
    If target is false, uses current vel value.
*/
class Entity_Travel : public EntityInterface {
    void _onExecEntity() override;
public:
    Entity_Travel();
    glm::vec3 vel;
    float accel_speed;
    float max_speed;
    bool target;
    glm::vec3 pos;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class Entity_Lifetime : public EntityInterface {
    void _onExecEntity() override;
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
    void _onCollisionEntity(EntityInterface* other) override;
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
    void _onExecEntity() override;
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

    Resource<Entity_FireParticle> resource_FireParticle;
    Resource<Entity_FireBall> resource_FireBall;
    Resource<Entity_Shooter> resource_Shooter;
    Resource<Entity_Chaser> resource_Chaser;
    Resource<Entity_LightBall> resource_LightBall;
    Resource<Entity_LightParticle> resource_LightParticle;
    Resource<Entity_Player> resource_Player;
    Resource<Entity_Being> resource_Being;
    Resource<Entity_Hitbox> resource_Hitbox;
    Resource<Entity_AcquirePlayer> resource_AcquirePlayer;
    Resource<Entity_Travel> resource_Travel;
    Resource<Entity_Lifetime> resource_Lifetime;
    Resource<Entity_Pickup> resource_Pickup;
    Resource<Entity_RepeatSpawn> resource_RepeatSpawn;

    std::unordered_map<std::string, int> inventory;

    EntityInterface* player;
    int game_state;
    float progress;

    int spawn_cooldown;
    std::vector<std::string> enemy_pool;
};

extern GlobalState globalstate;

#endif