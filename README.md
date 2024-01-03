# Engine

A simple rendering and scripting engine using an OpenGL backend. It is
designed to be easy to use, with as little development overhead as
possible. It allows the use of 3D models, but is currently largely
oriented towards 2D models, and the current graphics system only supports
2D quads.

Still in very heavy development and testing. Most of the code present
contains many debugging artifacts.

## Design

The core library contains a set of components which interact with one
another to give the engine its form desired by the user. The operative
units of the system are scripts, entities, and objects. These represent 
graphical and physical presences in the engine, which are accessed and 
modified by mechanisms like executors and managers.

The software design is a mostly linear hierarchy, with the form as follows:

![Software diagram.](https://i.imgur.com/3QwxJPG.png)

**Scripts**, **ScriptManagers**, and **Executors** form the bottom layer of the system.
These are units of execution used by the executor to embody a scripting
mechanism. The Script class is designed to be inherited. ScriptManagers will
automatically handle Script memory for the user, as well as associate them with
an executor.

**Entities** and **EntityManagers** form the next layer. Entities are Scripts with a **Quad**,
a graphical presence controlled by an existing **GLEnv**. The GLEnv component
encapsulates the OpenGL graphical backend for the system, managing data buffers;
this is primarily done through wrappers defined in the ``glutil.hpp`` module.

**Objects** and **ObjectManagers** form the uppermost layer. Objects are Entities with a
**Box**, a physical presence controlled by an existing **PhysEnv**. The PhysEnv component
facilitates physics-related simulations, i.e. collision detection based on the
current state of all contained Boxes.

## Descriptions & Usage

### Scripts, Entities, and Objects

**Scripts** represent an interface that allows the user specify the behavior of methods
controlled by an **Executor**:

- `init()`: will be called when the Script is queued for execution in an Executor and 
            the method `runExec()` is called, only the first time.
- `base()`: will be called every time the Script is queued for execution in an Executor 
            and the method `runExec()` is called.
- `kill()`: will be called every time the Script is queued for killing in an Executor 
          and the method `runKill()` is called. `base()` will not be called after this 
          occurs.

The Scripts contain various flags that dictate this behavior set and unset by Executors, 
that can be manually controlled by the user as well.

**Entities** are Scripts that contain a **Quad**. They must be provided a reference to an instance
of **GLEnv** to allow their `init()`, `base()`, and `kill()` methods to be called by an Executor.
They can then freely generate and remove their Quad from their provided GLEnv within their user-defined
behaviors. Note that their interface instead specifies `initEntity()`, `baseEntity()`, and `killEntity()`
to be overridden instead, which each call the former three.

**Objects** are Entities that contain a **Box**. Just like Entities, they must be provided a 
reference to an instance of **PhysEnv** to allow their `init()`, `base()`, and `kill()` methods to work.
They can then freely generate and remove their Box from their provided PhysEnv within their user-defined
behaviors. Their interface specifies `initObject()`, `baseObject()`, and `killObject()` to be 
overridden.

### Executors, GLEnvs and PhysEnvs

**Executors** represent a "scripting" mechanism. Given instances of Scripts, they can then be 
enqueued via `queueExec(int)` and `queueKill(int)`, and the Executor can subsequently perform 
calls on the Script methods `init()`, `base()`, and `kill()` through calls to the methods 
`runExec()` and `runKill()`. The Executors do not own any memory; they simply take references
to existing Scripts. This allows faster book-keeping and internal data management.

**GLEnvs** represent a graphical environment. They simplify an interface to OpenGL constructs,
including data buffers, Projection, View, and Transformation matrices, and texture data.
GLEnvs can internally "generate" **Quads** via `genQuad(...)`, which the user can obtain references 
to and manipulate. These Quads are then used for rendering, done with the `draw()` command.
GLEnv instances own the Quads that they generate.

**PhysEnvs** represent a physical environment. They embody a physical space within which
**Boxes** exist, which the PhysEnv can perform collision detection and other physics-related
computations with. Boxes are "generated" via `genBox(...)`. In this call, Boxes can also be
given a callback to be executed on collision. Detection is done on all existing and owned Boxes
with `detectCollision()`.

### Managers

**Managers** exist at each primary layer of the system, to provide the user with a simpler
means of managing **Script**, **Entity**, and **Object** instances, as well as to correlate them
with their corresponding controllers (**Executor**, **GLEnv**, and **PhysEnv** instances).

Managers can be given an "allocator"; this is a function that allocates an instance of a Script,
Entity, or Object on the heap and provides that address to the Manager. It must have the signature
`Script* allocate(void)` (or a pointer to some subclass of Script as the return value). The Manager
will own and manage the memory returned by this function. The user can also "bind" various flags 
to this function that will control what the Manager does upon invoking this method.

The **ScriptManager** class specifies the following fields to bind to an allocator:

- `const char *name`: name to associate with the allocator.
- `int type`: internal value tied to Script for client use (***may be deprecated soon***).
- `bool force_scriptsetup`: the Script will be set up with a stored Executor reference when spawning it.
- `force_enqueue`: enqueues this Script into a stored Executor reference when spawning it.
- `force_removeonkill`: removes this Script from this Manager when it is killed by the Executor.

The **EntityManager** subclass specifies the following additional fields:

- `force_entitysetup`: the Entity will be set up with a stored GLEnv reference when spawning it.
- `animation`: name of animation to give to AnimationState of spawned Entity, from provided Animation map 
               (see below for more details).

The **ObjectManager** subclass further specifies the following additional field:

- `force_objectsetup`: the Object will be set up with a stored PhysEnv reference when spawning it.

Note that an EntityManager can contain both Entities and Scripts. An ObjectManager can contain Objects,
Entities and Scripts.

### Animation Configuration

For finer control of texture data, an animation-oriented module is included. This provides a 
means of defining and loading texture-based values from JSON files.

The model for an animation is defined as a set of **Frames**, which contain values and durations, 
forming a **Cycle**; a set of **Cycles** forms an **Animation**. An **AnimationState** simplifies 
book-keeping for stepping through and accessing the data stored in an Animation that it contains 
a reference of.

The syntax of an animation configuration contained in a JSON file is as follows:

    {
        "name" : "example",
        "cycles" : {
            "cyclename1" : {
                "frames" : {
                    "framename1" : {
                        "texpos" : [0.0, 1.0],
                        "texsize" : [2.0, 3.0],
                        "offset" : [4.0, 5.0, 6.0],
                        "duration" : 4
                    },
                    "framename2" : {
                        "texpos" : [7.0, 8.0],
                        "texsize" : [9.0, 10.0],
                        "offset" : [11.0, 12.0, 13.0],
                        "duration" : 8
                    }
                    // ...
                },
                loop : true
            },
            "cyclename2" : {
                "frames" : {
                    "framename3" : {
                        // ...
                    }
                    // ...
                },
                loop : false
            }
            // ...
        }
   }

The "name" field is what key the configuration will be stored with in the map. 

The "cycles" field is a collection of objects that can each have any name, and correspond to 
individual cycles. Each cycle should contain a field called "frames", which is another collection
of objects that can also have any name, and correspond to individual frames; cycles additionally 
have a "loop" field which controls whether the AnimationState should loop or not on the frames.

Each frame should contain a "texpos", "texsize", "offset", and "duration" field, specifying
certain texture data values and values for use by Animation and AnimationState instances.

The function `loadAnimations(const char*)` loads Animations from a directory and returns a
map that an **EntityManager** can store a reference to. **Entities** contain AnimationStates,
which are used for writing to their **Quad** as their `baseEntity()` is called.

### Filter Configuration

To better control what should and shouldn't collide with each other in existing PhysEnv
instances, a filtering module is included. Like the animation module, the user can use this
to load collision filtering configurations from JSON files.

**Filters** simply contain a value, a whitelist, and a blacklist. Calling `pass(int)` on
a filter will return true or false depending on the state of the lists within the Filter.
**FilterStates** contain a reference to a Filter that detach access from the actual Filter
memory.

The syntax of an animation configuration contained in a JSON file is as follows:

    {
        "name" : "testname",
        "id" : 0,
        "whitelist" : [1, 2, 3, 4],
        "blacklist" : [3]
    }


The "name" field is what key the configuration will be stored with in the map. 

The "id" field is the individual value of the filter. The "whitelist" and "blacklist"
fields are arrays of integer for the filter to use when attempting to pass values.

The function `loadFilters(const char*)` loads Filters from a directory and returns a
map that an **ObjectManager** can store a reference to. **Objects** contain FilterStates,
which are used for filtering collisions when detection occurs.

Note that this module is currently not in use.

## Additional Notes

The top-level `src/` directory contains a demonstration of the core library, which
is being used for very basic testing and rapidly changes. It features the setup of the
engine to quickly spawn several Objects subjected to gravity.

This is a small personal project and was made for personal use. While it will be in
continuous development, updates will not be consistent and the present code may not
always be functional.

## License

This software is licensed under the MIT License.