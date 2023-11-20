#pragma once

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "../glenv/src/glenv.hpp"
#include "../executor/src/executor.hpp"

class EntityManager;

class Entity : public Script {
    friend EntityManager;

    // environmental references
    GLEnv *_glenv;

    // sprite objects
    Quad *_quad;
    Frame *_frame;

    // sprite variables/flags
    int _quad_id;
    bool _first_step;

    bool _glenv_ready;
    bool _quad_ready;

    // controllable variables
    glm::vec3 _visualpos;
    AnimationState _animstate;

    // Manager instance that owns this Entity; maintained by Manager
    EntityManager *_entitymanager;

    // called by execution environment
    void _init();
    void _base();
    void _kill();

protected:

    virtual void _initEntity();
    virtual void _baseEntity();
    virtual void _killEntity();

public:
    Entity();
    virtual ~Entity();

    /* Sets the entity up with graphics and animation resources. This
       enables the use of the genQuad(), getQuad(), and eraseQuad()
       methods.
    */
    void entitySetup(GLEnv *glenv, Animation *animation);

    glm::vec3 getVisPos();
    void setVisPos(glm::vec3 newpos);
    AnimationState &getAnimState();

    void genQuad(glm::vec3 pos, glm::vec3 scale);
    void eraseQuad();
    Quad *getQuad();

    EntityManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

/* Class to manage and control internal instances of Entities. 
   Can also be given an executor and glenv instance to automatically pass
   instances to these mechanisms.
*/
class EntityManager : public ScriptManager {
public:
    // struct holding entity information mapped to a name
    struct EntityType {
        bool _force_entitysetup;
        std::string _animation_name;
        std::function<Entity*(void)> _allocator = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed entity
    struct EntityValues {
        Entity *_entity_ref;
    };

protected:
    // internal variables for added entities and existing entities
    std::unordered_map<std::string, EntityType> _entitytypes;
    std::vector<EntityValues> _entityvalues;

    GLEnv *_glenv;
    std::unordered_map<std::string, Animation> *_animations;

    void _entitySetup(Entity *entity, EntityType &entitytype, ScriptType &scripttype, int id);
    void _entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues);
public:
    EntityManager(int maxcount);
    virtual ~EntityManager();
    
    bool hasEntity(const char *entityname);
    Entity *getEntity(int id);
    virtual int spawnScript(const char *scriptname);
    virtual int spawnEntity(const char *entityname);
    void addEntity(std::function<Entity*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name);
    void remove(int id);

    void setGLEnv(GLEnv *glenv);
    void setAnimations(std::unordered_map<std::string, Animation> *animations);
};

#endif