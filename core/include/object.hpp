#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "entity.hpp"
#include "physenv.hpp"

class ObjectManager;

/* class Object
   Represents an Entity that contains a box. objectSetup() must be called for the
   script methods _initEntity(), _baseEntity(), and _killEntity() to do anything.
*/
class Object : public Entity {
    friend ObjectManager;

    // environmental references
    PhysEnv *_physenv;
    Filter *_filter;

    // box object
    Box *_box;

    // box variables/flags
    int _box_id;
    bool _collision_elastic;

    bool _physenv_ready;
    bool _box_ready;

    // Manager instance that owns this Object; maintained by Manager
    ObjectManager *_objectmanager;

    void _initEntity();
    void _baseEntity();
    void _killEntity();

protected:
    /* Functions to be overridden by children.
       - _initObject() is called by _initEntity(). _initEntity() is called on execution, only for the first time the object is queued.
       - _baseObject() is called by _baseEntity(). _base() is called on execution, each time the object is queued.
       - _killObject() is called by _killEntity(). _kill() is called on erasure.
       - _collisionObject() is called by a PhysEnv when collision with this object's box is detected. This is supplied to the box on
         calling genBox().
    */
    virtual void _initObject();
    virtual void _baseObject();
    virtual void _killObject();
    virtual void _collisionObject(Box *box);

public:
    Object();
    virtual ~Object();
    
    /* Used to control whether elastic velocity change after collision should occur.
       Does nothing if collision is disabled.
    */
    bool getElastic();
    void setElastic(bool elastic);

    /* Sets the entity up with physics resources. This
       enables the use of the genBox(), getBox(), and eraseBox()
       methods.
    */
    void objectSetup(PhysEnv *physenv, Filter *filter);

    void genBox(glm::vec3 position, glm::vec3 dimensions, glm::vec3 velocity);
    void removeBox();
    Box *getBox();

    ObjectManager *getManager();
};

// --------------------------------------------------------------------------------------------------------------------------

/* class ObjectManager
   Manages and controls internal instances of Objects. Can also be given
   Executor, GLEnv and PhysEnv instance to automatically pass Object instances 
   to these mechanisms.
*/
class ObjectManager : public EntityManager {
public:
    // struct holding Object information mapped to a name
    struct ObjectInfo {
        std::string _filter_name;
        std::function<Object*(void)> _allocator = nullptr;
        std::function<void(Object*)> _spawncallback = nullptr;
    };

    // struct holding IDs and other flags belonging to the managed Object
    struct ObjectValues {
        Object *_object_ref;
    };

protected:
    // internal variables for added Objects and existing Objects
    std::unordered_map<std::string, ObjectInfo> _objectinfos;
    std::vector<ObjectValues> _objectvalues;

    PhysEnv *_physenv;
    std::unordered_map<std::string, Filter> *_filters;

    // internal methods called when spawning Objects and removing them, using and setting
    // manager lifetime and Object runtime members
    void _objectSetup(Object *object, ObjectInfo &objectinfo, EntityInfo &entityinfo, ScriptInfo &scriptinfo, int id);
    void _objectRemoval(ObjectValues &objectvalues, EntityValues &entityvalues, ScriptValues &scriptvalues);

public:
    /* maxcount - maximum number of Objects/Entities/Scripts to support in this instance */
    ObjectManager(int maxcount);
    ~ObjectManager();

    /* Returns true if the provided Object name has been previously added to this manager. */
    bool hasObject(const char *objectname);
    /* Returns a reference to the spawned Object corresponding to the provided ID, if it exists. */
    Object *getObject(int id);

    /* Spawns a Script using a name previously added to this manager, and returns its ID. This
       will invoke scriptSetup() if set to do so from adding it.
    */
    int spawnScript(const char *scriptname);
    /* Spawns an Entity using a name previously added to this manager, and returns its ID. This
       will invoke entitySetup() and scriptSetup() if set to do so from adding it.
    */
    int spawnEntity(const char *entityname);
    /* Spawns an Ontity using a name previously added to this manager, and returns its ID. This
       will invoke objectSetup(), entitySetup() and scriptSetup() if set to do so from adding it.
    */
    int spawnObject(const char *objectname);

    /* Adds an Object allocator with initialization information to this manager, allowing its given
       name to be used for future spawns.
       - allocator - function pointer referring to function that returns a heap-allocated Object
       - name - name to associate with the allocator
       - group - value to associate with all instances of this Object
       - force_enqueue - enqueues this Object into the provided Executor when spawning it
       - force_removeonkill - removes this Object from this manager when it is killed
       - animation_name - name of animation to give to AnimationState of spawned Object, from provided Animation map
       - filter_name - name of filter to give to FilterState of spawned Object's Box, from provided Filter map
       - spawn_callback - function callback to call after Object has been spawned and setup
    */
    void addObject(std::function<Object*(void)> allocator, const char *name, int group, bool force_enqueue, bool force_removeonkill, const char *animation_name, const char *filter_name, std::function<void(Object*)> spawn_callback);
    /* Removes the Object, Entity or Script associated with the provided ID. */
    void remove(int id);

    /* Sets the PhysEnv for this manager to use. */
    void setPhysEnv(PhysEnv *physenv);
    /* Sets the Filter map for this manager to use. */
    void setFilters(std::unordered_map<std::string, Filter> *filters);
};

#endif