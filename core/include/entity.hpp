#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "glenv.hpp"
#include "script.hpp"

typedef std::unordered_map<std::string, Animation> unordered_map_string_Animation_t;

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

    // controllable variables
    AnimationState _animationstate;

    // Manager instance that owns this Entity; maintained by Manager
    EntityManager *_entitymanager;

    // called by execution environment
    void _init() override;
    void _base() override;
    void _kill() override;

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
    Entity(Entity &&other);
    Entity();
    Entity(const Entity &other) = delete;
    virtual ~Entity();

    Entity& operator=(Entity &&other);
    Entity& operator=(const Entity &other) = delete;

    /* Sets the entity up with graphics and animation resources. This
       enables the use of the genQuad(), getQuad(), and eraseQuad()
       methods.
    */
    void entitySetup(GLEnv *glenv, Animation *animation);
    /* Removes GLEnv and Animation information stored. */
    void entityClear();

    AnimationState &getAnimState();
    void stepAnim();

    void genQuad(glm::vec3 pos, glm::vec3 scale);
    void removeQuad();
    Quad *getQuad();

    EntityManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class EntityManager
   Manages and controls internal instances of Entities. Can also be given 
   Executor and GLEnv instance to automatically pass Entity instances to these 
   mechanisms.

   It is undefined behavior to make method calls (except for uninit()) on instances 
   of this class without calling init() first.
*/
class EntityManager : public ScriptManager {
public:
    // struct holding Entity information mapped to a name
    struct EntityInfo {
        std::string _animation_name;
        std::function<Entity*(void)> _allocator = nullptr;
        std::function<void(Entity*)> _spawn_callback = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed Entity
    struct EntityValues {
        Entity *_entity_ref;
    };

    // _valid flag is used to prevent instance from being spawned as an Entity
    struct EntityEnqueue {
       bool _valid;
       glm::vec3 _entity_pos;
    };

protected:
    // internal variables for added Entities and existing Entities
    std::unordered_map<std::string, EntityInfo> _entityinfos;
    std::vector<EntityValues> _entityvalues;
    std::queue<EntityEnqueue> _entityenqueues;

    GLEnv *_glenv;
    unordered_map_string_Animation_t *_animations;

    // internal methods called when spawning Entities and removing them, using and setting
    // manager lifetime and Entity runtime members
    void _entitySetup(Entity *entity, EntityInfo &entityinfo, ScriptInfo &scriptinfo, unsigned id, glm::vec3 entity_pos);
    void _entityRemoval(EntityValues &entityvalues, ScriptValues &scriptvalues);

    // initialize/uninitialize only EntityManager members
    void _entityManagerInit(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations);
    void _entityManagerUninit();
public:
    /* Calls init() with the provided arguments. */
    EntityManager(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor);
    EntityManager(EntityManager &&other);
    EntityManager();
    EntityManager(const EntityManager &other) = delete;
    virtual ~EntityManager();

    EntityManager &operator=(EntityManager &&other);
    EntityManager &operator=(const EntityManager &other) = delete;

    void init(unsigned max_count, GLEnv *glenv, unordered_map_string_Animation_t *animations, Executor *executor);
    void uninit();

    /* Spawns a Script using a name previously added to this manager, and returns its ID. This
       will invoke scriptSetup().
    */
    virtual unsigned spawnScript(const char *script_name) override;
    /* Spawns an Entity using a name previously added to this manager, and returns its ID. This
       will invoke entitySetup() and scriptSetup().
    */
    virtual unsigned spawnEntity(const char *entity_name, glm::vec3 entity_pos);
    /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
    virtual void spawnScriptEnqueue(const char *script_name) override;
    /* Enqueues a Entity to be spawned when calling runSpawnQueue(). */
    virtual void spawnEntityEnqueue(const char *script_name, glm::vec3 object_pos);
    /* Spawns all Scripts (or sub classes) queued for spawning with spawnScriptEnqueue() or spawnEntityEnqueue(). */
    virtual std::vector<unsigned> runSpawnQueue() override;
    /* Removes the Entity or Script associated with the provided ID. */
    void remove(unsigned id);

    /* Adds an Entity allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - function pointer referring to function that returns a heap-allocated Entity
       - name - name to associate with the allocator
       - group - value to associate with all instances of this Entity
       - force_executorenqueue - enqueues this Entity into the provided Executor when spawning it
       - force_removeonkill - removes this Entity from this manager when it is killed
       - animation - name of animation to give to AnimationState of spawned Entity, from provided Animation map
       - spawn_callback - function callback to call after Entity has been spawned and setup
    */
    void addEntity(std::function<Entity*(void)> allocator, const char *name, int group, bool force_executorenqueue, bool force_removeonkill, const char *animation_name, std::function<void(Entity*)> spawn_callback);

    /* Returns true if the provided Entity name has been previously added to this manager. */
    bool hasAddedEntity(const char *entity_name);
    /* Returns a reference to the spawned Entity corresponding to the provided ID, if it exists. */
    Entity *getEntity(unsigned id);


};

#endif