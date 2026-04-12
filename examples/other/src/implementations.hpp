#ifndef IMPLEMENTATIONS_HPP_
#define IMPLEMENTATIONS_HPP_

#include "resource.hpp"
#include "../../../core/include/glfwinput.hpp"

const float diag_factor = glm::sin(glm::radians(45.0f));

struct GlobalResources;

class SpellInterface : public EntityScriptInterface, public Resource<GlobalResources> {
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
    virtual void _initSpell() = 0;
    virtual void _execSpell() = 0;
    virtual void _killSpell() = 0;
    virtual void _updateSpell() = 0;
public:
    SpellInterface();
};

// --------------------------------------------------------------------------------------------------------------------------

class Spell_LightBallSpell : public SpellInterface {
    void _initSpell() override;
    void _execSpell() override;
    void _killSpell() override;
    void _updateSpell() override;
public:
    Spell_LightBallSpell();
};

// --------------------------------------------------------------------------------------------------------------------------

enum CastType {CASTTYPE_TOME, CASTTYPE_STAVE};
/* struct Castable
   source - can be used to avoid casting twice
   type - type of cast
   spell_entity_name - EntityScript name to be spawned
   cast_time - duration for spell_name to be used
   pos - cast position; may not be used
   dir - cast direction; may not be used
*/
struct Castable {
    Castable* source;
    CastType type;
    std::string spell_entity_name;
    int cast_time;
    glm::vec3 pos;
    glm::vec3 dir;
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Player 
    Player script.
*/
class ES_Player : public EntityScriptInterface, public Resource<GlobalResources> {
    std::list<Castable> _castables;
    std::list<Castable> _casts;
    float _hurt_cooldown_max;
    float _hurt_cooldown;
    float _hitbox_cooldown_max;
    float _hitbox_cooldown;
    float _speed;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity* other, std::string message) override;
    void _collide(Entity* other) override;
public:
    ES_Player();
    void checkCasts();
};

// --------------------------------------------------------------------------------------------------------------------------

/*
    Entity Script Chaser
    Chases assigned target directly.
*/
class ES_Chaser : public EntityScriptInterface, public Resource<GlobalResources> {
    Entity *_target;
    void _initEntity() override;
    void _execEntity() override;
    void _killEntity() override;
    void _updateEntity() override;
    void _receive(Entity *other, std::string message) override;
    void _collide(Entity *other) override;
public:
    ES_Chaser();
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

/* struct Global Resources
   Aggregates resources for classes with Resource<GlobalResources> inherited to access.
*/
struct GlobalResources {
    GlobalResources(EntityManager* entitymanager, EntityScriptExecutor* entityscriptexecutor, GLFWInput* glfwinput);

    EntityManager* manager;
    EntityScriptExecutor* executor;
    GLFWInput* input;
    
    ScriptContainer<ES_Player> container_Player;
    ScriptContainer<ES_Chaser> container_Chaser;
    ScriptContainer<ES_Lifetime> container_Lifetime;
    ScriptContainer<Spell_LightBallSpell> container_LightBallSpell;
};

#endif