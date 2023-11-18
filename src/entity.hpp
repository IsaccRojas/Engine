#pragma once

#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "../glenv/src/glenv.hpp"
#include "../executor/src/executor.hpp"

class Entity : public Script {
    // environmental references
    GLEnv *_glenv;

    // sprite objects
    Quad *_quad;
    Frame *_frame;

    // sprite variables/flags
    int _quad_id;
    bool _first_step;

    bool _setup_ready;
    bool _quad_ready;

    // controllable variables
    glm::vec3 _visualpos;
    AnimationState _animstate;

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

    void genQuad(glm::vec3 pos, glm::vec3 scale);
    void eraseQuad();
    Quad *getQuad();
};

#endif