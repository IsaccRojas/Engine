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
units of the system are scripts, quads, and colliders. These represent
units of execution, graphical presences, and physical presences in the 
engine, which are accessed and modified by mechanisms like executors.

The software design is a mostly polymorphic hierarchy, with the form as follows:

![Software diagram.](https://i.imgur.com/VflSj2F.png)

**Scripts** and **Executors** form the bottom layer of the system, and embody 
a scripting mechanism. The Script class is an interface modeling an execution 
lifetime. The **AllocatorInterface** must be implemented, and an instance can be 
provided to an **Executor** instance and mapped to a name. This enables the **Executor**
to instantiate and manage Script subtypes, "executing" them through a queueing
API.

The **GLEnv** component encapsulates the OpenGL graphical backend for the system, 
managing data buffers and graphical presences defined as **Quads**; this is primarily 
done through wrappers defined in the ``glutil.hpp`` module.

The **PhysSpace** component facilitates basic physics-related simulations, i.e. collision 
detection based on the current state of all contained **Colliders**.

Additionally included in the library is a templated **ProvidedType**, **Provider**, and **Receiver** 
scheme that implements **AllocaterInterface** to enable simplistic runtime retrieval of allocated 
instances of subtypes implementing the Script class.

An additional layer for subtypes of Scripts that contain references to GLEnv and PhysSpace
instances is also provided, called **Entities**.

Simplistic wrappers for GLFW window and input handling are also included.

## Descriptions & Usage

### Scripts, Quads, Colliders and Allocators

**Scripts** represent an interface that allows the user specify the behavior of methods
controlled by an **Executor**, with the following exposed virtual methods:

- `Script::_init()`: will be called when the Script is queued for execution in an Executor, only the
            first time it is executed.
- `Script::_base()`: will be called every time the Script is queued for execution in an Executor 
            and execution is called.
- `Script::_kill()`: will be called every time the Script is queued for killing in an Executor 
            and the killing is called. `Script::_base()` will not be called after this occurs.

The Scripts contain various flags that dictate this behavior set and unset by Executors. Executors
must be provided with an instantiation of an implementation of **AllocatorInterface**, which can be
mapped to a string name and callbacks.

**Entities** are Scripts that have access to **GLEnv** and **PhysSpace** resources. **EntityExecutors** must 
be passed a reference of **GLEnv**, **PhysSpace**, an **Animation** map and a **Filter** map. `Entity::_initEntity()`,
`Entity::_baseEntity()`, and `Entity::_killEntity()` are instead exposed, which wrap the ``Script::_init()``,
``Script::_base()`` and ``Script::_kill()`` methods.

### Executors, GLEnvs and PhysSpaces

**Executors** represent a "scripting" mechanism. When provided with instances of **AllocatorInterface** mapped
to a name, they can enqueue "spawns" of **Scripts** with the names added, and enqueue active Script instances
for execution or to be killed.

The following fields can be mapped to stored AllocatorInterface references, via ``Executor::add()``:
- `const char *name`: name to associate with the allocator.
- `int group`: internal value tied to Script for client use, and with getAllByGroup(int) method.
- `bool removeonkill`: removes this Script from this Executor when it is killed.
- `std::function<void(unsigned)> spawn_callback` - function callback to invoke after Script has been instantiated with this allocator, and setup.
- `std::function<void(unsigned)> remove_callback` - function callback to call before this Script is removed from the Executor.

After adding an AllocatorInterface to an Executor, it can be invoked via calls to ``Executor::enqueueSpawn()`` passing
the corresponding mapped name as an argument, as well as specifying an initial execution queue to insert the instantiated
Script into. A tag argument can also be passed, which will later be provided to the corresponding AllocatorInterface for
user interpretation. A subsequent call to ``Executor::runSpawnQueue()`` can be invoked to perform all previously enqueued spawns.

Executor-instantiated Scripts are assigned an ID that is unique for the duration of their lifetime within the Executor; that
is, until Executor::remove() is invoked with their ID as an argument, which can be called by killing the Script
if `removeonkill` is set for its allocator mapping.

Active Scripts can be controlled via the following API:
- ``Executor::enqueueExec(int id, unsigned queue)`` - Enqueues Script corresponding to ID argument for execution, into the specified queue.
- ``Executor::enqueueKill(int id)`` - Enqueues Script corresponding to ID argument to be killed.
- ``Executor::runExecQueue(unsigned queue)`` - Executes all Scripts previously enqueued into the provided execution queue.
- ``Executor::runKillQueue()`` - Kills all Scripts previously enqueued to be killed.

**GLEnvs** represent a graphical environment. They simplify an interface to OpenGL constructs, including data buffers, 
Projection, View, and Transformation matrices, and texture data. GLEnvs can internally instantiate **Quads** via 
`GLEnv::genQuad()`, which the user can obtain references to and manipulate. These Quads can have their representative data
written to the underlying OpenGL API via calls to ``GLEnv::update()`` methods either on the Quads or on the owning GLEnv instance.
Rendering can be performed with the `GLEnv::draw()` method. Quads can additionally be passed a reference to **Animation** data,
which can then be used to write to its texture-related buffers based on the Animation configuration being referenced, via their
**AnimationState** preserving per-instance state information of the Animation being used.

**PhysSpaces** represent a physical environment. They embody a physical space within which **Colliders** exist, which the PhysSpace
can perform collision detection and other physics-related computations with. The PhysSpace class is templated, and will support collision
between subtypes that implement the Collider interface. A PhysSpace instance can internally instantiate Colliders via
`PhysSpace<T>::push()`. In this call, Colliders can also be given a callback to be executed on collision. Detection is done on all 
owned Colliders with `PhysSpace<T>::detectCollision()`. Colliders can additionally be passed a reference to **Filter** data, which will be
used by the detection method to determine if the collision should take place between a pair of contained Colliders, via their
**FilterState** preserving per-instance state information of the Filter being used. Implementations of the Collider interface for 
axis-aligned boxes and spheres are included.

**EntityExecutors** facilitate the execution of the **Entity** subtypes, by extending the Executor definition with resources that
its owned Entity instances will have access to.

### ProvidedTypes, Providers, ProvidedAllocators, and Receivers

The **ProvidedType**, **Provider**, **ProvidedAllocator** and **Receiver** class templates are included in the library as an
extension of the **AllocatorInterface** mechanism, to facilitate simple routing of references between **Executor**-managed 
**Script** covariants.

ProvidedAllocators are implementations of AllocatorInterface that extend the allocation method with the passing of the allocated
instances (which must inherit ProvidedType) to a containing Provider.

Providers store ProvidedAllocator references in order to have their allocations stored here for broadcasting and getting. The 
tag argument of the ``AllocatorInterface::_allocate()`` method in this scheme's case is interpreted as a channel. On invocation
of this method on any of the contained ProvidedAllocators, the ProvidedType reference is stored, as well as broadcasted to any 
subscribed Receivers (via ``Provider<T>::subscribe()``) with a matching channel value through ``Receiver<T>::_receive()``.

A class inheriting ProvidedType is simply extended to store a corresponding reference to a Provider, so that, given
that the Provider stores this type, it can be removed on destruction or earlier. The class inheriting ProvidedType
must provide itself as the template argument.

A class inheriting Receiver is extended to enable reception and getting of instances of a class inheriting ProvidedType,
and contains state related to a Provider. It may override ``Receiver<T>::_receive()`` to be broadcasted any spawns tagged
with its channel from subscribed Providers, and retrieve any active ProvidedTypes via ``Receiver<T>::getAllProvided()``.

Note that these four template definitions are codependent, and some declaration of ProvidedType<T>, Provider<T>, ProvidedAllocator<T>, 
and Receiver<T> must exist for the same ``T`` if any one exists. In other words, if something is provided, it must be received; 
and vice versa.

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
map that an **EntityExecutor** instance can store a reference to. **Entities** can directly access
this map via their contained reference to their owner.

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
map that an **EntityExecutor** instance can store a reference to. **Entities** can directly access
this map via their contained reference to their owner.

## Additional Notes

The top-level `core/` directory contains the core library. The `examples/` directory contains
a small example game demonstrating usage of the library, of which a binary compiled on a 64-bit
Windows 10 architecture is available in Releases. It is being used for basic testing and the
source code may rapidly change with respect to the available binary.

This is a small personal project and was made for personal use. While it will be in
continuous development, updates will not be consistent and the present code may not
always be functional.

## License

This software is licensed under the MIT License.
