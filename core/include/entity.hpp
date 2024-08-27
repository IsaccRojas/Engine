#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "glenv.hpp"
#include "script.hpp"

class EntityManager;

/* class Entity
   Represents a Script that contains a Quad. entitySetup() must be called for the
   script methods _init(), _base(), and _kill() to do anything.
*/
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
    AnimationState _animstate;

    // Manager instance that owns this Entity; maintained by Manager
    EntityManager *_entitymanager;

    // called by execution environment
    void _init();
    void _base();
    void _kill();

protected:
    /* Functions to be overridden by children.
       - _initEntity() is called by _init(). _init() is called on execution, only for the first time the entity is queued.
       - _baseEntity() is called by _base(). _base() is called on execution, each time the entity is queued.
       - _killEntity() is called by _kill(). _kill() is called on erasure.
    */
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

    AnimationState &getAnimState();
    void stepAnim();

    void genQuad(glm::vec3 pos, glm::vec3 scale);
    int removeQuad();
    Quad *getQuad();

    EntityManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityManager
   Manages and controls internal instances of Entities. Can also be given 
   Executor and GLEnv instance to automatically pass Entity instances to these 
   mechanisms.
*/
class EntityManager : public ScriptManager {
public:
    // struct holding Entity information mapped to a name
    struct EntityType {
        bool _force_entitysetup;
        std::string _animation_name;
        std::function<Entity*(void)> _allocator = nullptr;
        std::function<void(Entity*)> _spawncallback = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed Entity
    struct EntityValues {
        Entity *_entity_ref;
    };

protected:
    // internal variables for added Entities and existing Entities
    std::unordered_map<std::string, EntityType> _entitytypes;
    std::vector<EntityValues> _entityvalues;

    GLEnv *_glenv;
    std::unordered_map<std::string, Animation> *_animations;

    // internal methods called when spawning Entities and removing them, using and setting
    // manager lifetime and Entity runtime members
    void _entitySetup(Entity *entity, EntityType &entitytype, ScriptType &scripttype, int id);
    void _entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues);

public:
    /* maxcount - maximum number of Entities/Scripts to support in this instance */
    EntityManager(int maxcount);
    virtual ~EntityManager();
    
    /* Returns true if the provided Entity name has been previously added to this manager. */
    bool hasEntity(const char *entityname);
    /* Returns a reference to the spawned Entity corresponding to the provided ID, if it exists. */
    Entity *getEntity(int id);

    /* Spawns a Script using a name previously added to this manager, and returns its ID. This
       will invoke scriptSetup() if set to do so from adding it.
    */
    virtual int spawnScript(const char *scriptname);
    /* Spawns an Entity using a name previously added to this manager, and returns its ID. This
       will invoke entitySetup() and scriptSetup() if set to do so from adding it.
    */
    virtual int spawnEntity(const char *entityname);

    /* Adds an Entity allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - function pointer referring to function that returns a heap-allocated Entity
       - name - name to associate with the allocator
       - type - internal value tied to Entity for client use
       - force_scriptsetup - invokes Script setup when spawning this Entity
       - force_enqueue - enqueues this Entity into the provided Executor when spawning it
       - force_removeonkill - removes this Entity from this manager when it is killed
       - force_entitysetup - invokes Entity setup when spawning this Entity
       - animation - name of animation to give to AnimationState of spawned Entity, from provided Animation map
       - spawn_callback - function callback to call after Entity has been spawned and setup
    */
    void addEntity(std::function<Entity*(void)> allocator, const char *name, int type, bool force_scriptsetup, bool force_enqueue, bool force_removeonkill, bool force_entitysetup, const char *animation_name, std::function<void(Entity*)> spawn_callback);
    /* Removes the Entity or Script associated with the provided ID. */
    void remove(int id);

    /* Sets the GLEnv for this manager to use. */
    void setGLEnv(GLEnv *glenv);
    /* Sets the Animation map for this manager to use. */
    void setAnimations(std::unordered_map<std::string, Animation> *animations);
};

#endif