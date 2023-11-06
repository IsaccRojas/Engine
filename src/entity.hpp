#pragma once

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "../glenv/src/glenv.hpp"
#include "../execenv/src/execenv.hpp"

class Entity : public Script {
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

#endif