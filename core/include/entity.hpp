#ifndef ENTITY_HPP_
#define ENTITY_HPP_

#include "animation.hpp"
#include "glenv.hpp"
#include "script.hpp"

typedef std::unordered_map<std::string, Animation> unordered_map_string_Animation_t;

/* class Entity
   Represents a Script that contains a Quad. entitySetup() must be called for the
   script methods _initEntity(), _baseEntity(), and _killEntity() to do anything.
*/
class Entity : public Script {
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

    // called by execution environment
    void _init() override;
    void _base() override;
    void _kill() override;

    void _entityRemove();
protected:
    /* Functions to be overridden by children.
       - _initEntity() is called by _init(). _init() is called on execution, only for the first time the entity is queued.
       - _baseEntity() is called by _base(). _base() is called on execution, each time the entity is queued.
       - _killEntity() is called by _kill(). _kill() is called on erasure.
    */
    virtual void _initEntity() = 0;
    virtual void _baseEntity() = 0;
    virtual void _killEntity() = 0;

public:
    Entity(Entity &&other);
    Entity();
    Entity(const Entity &other) = delete;
    virtual ~Entity();

    Entity& operator=(Entity &&other);
    Entity& operator=(const Entity &other) = delete;

    void stepAnim();

    AnimationState &getAnimState();
    Quad *getQuad();
};

// --------------------------------------------------------------------------------------------------------------------------

class EntityAllocatorInterface {
   friend EntityExecutor;
protected:
   /* Must return a heap-allocated instance of a covariant type of Entity. */
   virtual Entity *_allocate(int tag) = 0;
};

class EntityExecutor : public Executor {
public:
    // struct holding Entity information mapped to a name
    struct EntityInfo {
       EntityAllocatorInterface *_allocator;
       std::string _animation_name;
    };
    struct EntityEnqueue {
      bool _is_entity;
      glm::vec3 _pos;
    };

private:
    // internal variables for added Entityy information and enqueued Entities
    std::unordered_map<std::string, EntityInfo> _entityinfos;
    std::queue<EntityEnqueue> _entityenqueues;

    GLEnv *_glenv;
    unordered_map_string_Animation_t *_animations;

    // spawns an Entity using a name previously added to this manager, and returns its ID
    unsigned _spawnEntity(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
public:
    /* Calls init() with the provided arguments. */
    EntityExecutor(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations);
    EntityExecutor(EntityExecutor &&other);
    EntityExecutor();
    EntityExecutor(const EntityExecutor &other) = delete;
    virtual ~EntityExecutor() override;

    EntityExecutor &operator=(EntityExecutor &&other);
    EntityExecutor &operator=(const EntityExecutor &other) = delete;

    void init(unsigned max_count, unsigned queues, GLEnv *glenv, unordered_map_string_Animation_t *animations);
    void uninit();

    /* Adds a Entity allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - Reference to instance of class implementing EntityAllocatorInterface.
       - name - name to associate with the allocator
       - group - value to associate with all instances of this Entity
       - force_removeonkill - removes this Entity from this manager when it is killed
       - filter_name - filter name to associate with this name
       - spawn_callback - function callback to call after Entity has been spawned and setup
       - remove_callback - function callback to call before Entity has been removed
    */
    void add(EntityAllocatorInterface *allocator, const char *name, int group, bool force_removeonkill, std::string filter_name, std::function<void(unsigned)> spawn_callback, std::function<void(unsigned)>  remove_callback);

    /* Enqueues a Script to be spawned when calling runSpawnQueue(). */
    virtual void enqueueSpawn(const char *script_name, int execution_queue, int tag) override;
    /* Enqueues an Entity to be spawned when calling runSpawnQueue(). */
    void enqueueSpawn(const char *entity_name, int execution_queue, int tag, glm::vec3 pos);
    /* Spawns all Entities (or sub classes) queued for spawning with enqueueSpawn(). */
    virtual std::vector<unsigned> runSpawnQueue() override;
};

#endif