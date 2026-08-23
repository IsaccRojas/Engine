#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "../../../core/include/entity.hpp"
#include "../../../core/include/glfwstate.hpp"
#include "../../../core/include/glfwinput.hpp"

struct GlobalState;

// --------------------------------------------------------------------------------------------------------------------------

/*
    Assumes Tile and Collider are both squares and the same size
    Assumes Tile does not move
*/
class ES_Correction : public EntityScriptInterface {
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
public:
    ES_Correction();
    unsigned collider_index;
    float assist_speed;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    class ES_Player 
    Player script.
*/
class ES_Player : public EntityScriptInterface {
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _speed;
    glm::vec3 _last_input_dir;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
public:
    ES_Player();
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Lifetime
    Kills itself after assigned lifetime. Can be assigned a velocity to adjust its own position during its lifetime.
    Can also be assigned a target to follow position of. Overrides application of velocity.

    message "target" - assigns target Entity to copy position of
*/
class ES_Lifetime : public EntityScriptInterface {
    Entity* _target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Lifetime();
    unsigned lifetime;
    glm::vec3 vel;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Pickup
    Upon collision, increments the name on the GlobalResources' inventory map and kills itself

    std::string item_name - name to increment in GlobalResources' inventory
*/
class ES_Pickup : public EntityScriptInterface {
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Pickup();
    std::string item_name;
};

// --------------------------------------------------------------------------------------------------------------------------

struct TileInfo {
    int value;
    int quad_id_lower;
    int quad_id_upper;
};

struct MapInfo {
    glm::vec2 unit_pixel_dimensions;
    glm::vec2 coord_dimensions;
    glm::vec2 coord_origin;
    glm::ivec2 toCoords(glm::vec2 v);
    glm::vec2 toPixels(glm::ivec2 v);
    bool isValid(glm::vec2 v);
};

struct GlobalState {
    GlobalState();

    GLFWState glfwstate;
    GLFWInput input;

    std::unordered_map<std::string, Animation> animations;
    std::unordered_map<std::string, Filter> filters;
    EntityScriptExecutor executor;
    GLEnv glenv;
    CollisionSpace collisionspace;
    EntityManager manager;

    GenericProvidingEntityScriptAllocator<ES_Correction> allocator_Correction;
    GenericProvidingEntityScriptAllocator<ES_Player> allocator_Player;
    GenericProvidingEntityScriptAllocator<ES_Pickup> allocator_Pickup;
    GenericProvidingEntityScriptAllocator<ES_Lifetime> allocator_Lifetime;
    
    RefContainer<ES_Player> container_Player;
    RefContainer<ES_Lifetime> container_Lifetime;
    RefContainer<ES_Pickup> container_Pickup;

    MapInfo mapinfo;
    std::vector<std::vector<TileInfo>> map;
    std::unordered_map<std::string, int> inventory;
    bool stairs_entered;
    bool level_generated;
    bool level_clear_started;
};

extern GlobalState globalstate;

#endif