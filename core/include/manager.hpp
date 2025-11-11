#ifndef _MANAGER_HPP_
#define _MANAGER_HPP_

#include "script.hpp"
#include "glenv.hpp"
#include "physspace.hpp"

struct Scheme;
typedef std::unordered_map<std::string, Scheme> unordered_map_string_Scheme_t;

struct ScriptArgs {
    const char *script_name;
    int execution_queue;
    int tag;
};
struct QuadArgs {
    glm::vec3 pos;
    glm::vec3 scale;
    glm::vec4 color;
    DrawType type;
    const char *animation_name;
    glm::vec3 texpos;
    glm::vec2 texsize;
    GLfloat innerrad;
};
struct BoxArgs {
    Transform transf;
    glm::vec3 vel;
    std::function<void(Box*)> callback;
    const char *filter_name;
};
struct Scheme {
    std::list<ScriptArgs> _script_args;
    std::list<QuadArgs> _quad_args;
    std::list<BoxArgs> _box_args;
};

class Manager {
    Executor *_executor;
    GLEnv * _glenv;
    PhysSpace<Box> *_physspace_box;

    unordered_map_string_Scheme_t _schemes;

    bool _initialized = false;
public:
    Manager(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box);
    Manager();
    void init(Executor *executor, GLEnv *glenv, PhysSpace<Box> *physspace_box);
    void uninit();
    void addScheme(Scheme s, const char *name);
    void instScheme(const char *name);
};

#endif