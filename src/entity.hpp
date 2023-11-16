#pragma once

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "../glenv/src/glenv.hpp"
#include "../executor/src/executor.hpp"

class EntitySpawner;

class Entity : public Script {
    friend EntitySpawner;

    // environmental references
    GLEnv *_glenv;

    // sprite objects
    Quad *_quad;
    Frame *_frame;

    // sprite variables/flags
    int _quad_id;
    bool _first_step;

    bool _spawner_ready;
    bool _quad_ready;

    // called by execution environment
    void _init();
    void _base();
    void _kill();

protected:
    glm::vec3 _visualpos;
    AnimationState _animstate;

    virtual void _initEntity();
    virtual void _baseEntity();
    virtual void _killEntity();

public:
    Entity();
    ~Entity();

    /* Sets the entity up with graphics and animation resources. This
       enables the use of the genQuad(), getQuad(), and eraseQuad()
       methods.
    */
    void entitySetup(GLEnv *glenv, Animation *animation);

    void genQuad(glm::vec3 pos, glm::vec3 scale);
    void eraseQuad();
    Quad *getQuad();
};

// --------------------------------------------------------------------------------------------------------------------------

class EntitySpawner {
    // struct holding entity information mapped to a name
    struct _EntityType {
        std::function<Entity*(void)> allocator;
        std::string entityname;
    };

    // allocator variables
    std::unordered_map<std::string, _EntityType> _entitytypes;

public:
    /* ...
    */
    EntitySpawner();
    //TODO: write copy/move constr., destr.

    /* Maps a function returning a new entity to a string.
    */
    void add(std::function<Entity*(void)> allocator, const char *entityname);

    /* ...
    */
    bool has(const char *name);

    /* ...
    */
    Entity *spawn(const char *entityname);
};

#endif