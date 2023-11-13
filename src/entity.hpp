#pragma once

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "../glenv/src/glenv.hpp"
#include "../execenv/src/execenv.hpp"

class EntitySpawner;

class Entity : public Script {
    friend EntitySpawner;

    // environmental references
    GLEnv *_glenv;

    // sprite objects
    Quad *_quad;
    AnimationState _animstate;
    Frame *_frame;

    // sprite variables/flags
    int _quadid;
    bool _firststep;
    bool _ready;

    // called by spawner to properly set up internal values; _init(),
    // _base(), and _kill() do nothing until this is called
    void _setup(GLEnv *glenv, Animation *animation);

    // called by execution environment
    void _init();
    void _base();
    void _kill();

protected:
    glm::vec3 pos;

    virtual void _initEntity();
    virtual void _baseEntity();
    virtual void _killEntity();

public:
    Entity();
    ~Entity();

    Quad *quad();
};

class EntitySpawner {
    // struct holding entity information mapped to a name
    struct _EntityType {
        std::function<Entity*(void)> allocator;
        std::string entityname;
        std::string animationname;
    };

    // resources
    GLEnv *_glenv;
    std::unordered_map<std::string, Animation> _animations;
    bool _animloaded;

    // allocator variables
    std::unordered_map<std::string, _EntityType> _entitytypes;
public:
    /* ...
    */
    EntitySpawner(GLEnv *glenv);
    //TODO: write copy/move constr., destr.

    /* ...
    */    
    void loadAnimations(const char *directory);

    /* Maps a function returning a new entity to a string and animation.
    */
    int add(std::function<Entity*(void)> allocator, const char *entityname, const char *animationname);

    /* ...
    */
    Entity *spawn(const char *entityname);
};

#endif