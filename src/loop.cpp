#include "loop.hpp"

enum Type { T_BLOCK1, T_TEST1, T_TEST2, T_EFFECT1, T_EFFECT2, T_EFFECT3 };

class Block1 : public Object {
    void _initObject() {
        getBox()->dim = glm::vec3(192.0f, 32.0f, 0.0f);

        getQuad()->scale.v = glm::vec3(192.0f, 32.0f, 1.0f);
        getQuad()->pos.v = getBox()->pos;
    };

    void _baseObject() {};

    void _killObject() {
        removeBox();
        removeQuad();
    };

    void _collisionObject(Box *box) {}
};

class Test1 : public Object {
    int _i;
    int _lifetime;
    bool _collided;

    void _initObject() {
        getBox()->dim = glm::vec3(16.0f, 16.0f, 0.0f);

        getQuad()->scale.v = glm::vec3(16.0f, 16.0f, 1.0f);
        getQuad()->pos.v = getBox()->pos;

        setCorrection(true);
        
        _i = 0;
        _lifetime = 600;
        _collided = false;
    };

    void _baseObject() {
        getQuad()->pos.v = getBox()->pos;
        
        _i++;
        if (_i >= _lifetime)
            kill();
        else
            enqueue();
    };

    void _killObject() {
        Entity *effect = getManager()->getEntity(getManager()->spawnEntity("Effect1"));
        effect->getQuad()->pos.v = getBox()->pos;

        // spawn Test2s
        if (_collided) {
            for (int i = 0; i < 4; i++) {
                Object *test2 = getManager()->getObject(getManager()->spawnObject("Test2"));

                // set velocity of new test2s to opposite of this object's velocity with random magnitude, rotated randomly
                glm::vec3 newvel = getBox()->vel;
                newvel = glm::vec3(newvel.x, -1.0f * newvel.y, newvel.z) * (0.4f + (float(rand() % 50) / 100.0f));
                newvel = random_angle(newvel, 35.0f);

                test2->getBox()->vel = newvel;
                test2->getBox()->pos = getBox()->pos + newvel;
            }
        }

        removeBox();
        removeQuad();
    };

    void _collisionObject(Box *box) {
        /*
        if (obj->getType() == T_TEST1 || obj->getType() == T_TEST2)
            return;
        */
        _collided = true;
        kill();
    }

public:
    Test1() : Object() {}
};

class Test2 : public Object {
    int _i;
    int _lifetime;

    void _initObject() {
        getBox()->dim = glm::vec3(12.0f, 12.0f, 0.0f);

        getQuad()->scale.v = glm::vec3(16.0f, 16.0f, 1.0f);
        getQuad()->pos.v = getBox()->pos;

        _i = 0;
        _lifetime = 600;
    };

    void _baseObject() {
        getQuad()->pos.v = getBox()->pos;

        _i++;
        if (_i >= _lifetime)
            kill();
        else
            enqueue();
    };

    void _killObject() {
        Entity *effect = getManager()->getEntity(getManager()->spawnEntity("Effect2"));
        effect->getQuad()->pos.v = getBox()->pos;

        removeBox();
        removeQuad();
    };

    void _collisionObject(Box *box) {
        /*
        if (obj->getType() == T_TEST1 || obj->getType() == T_TEST2)
            return;
        */
        kill();
    }

public:
    Test2() : Object() {}
};

class Effect1 : public Entity {
    int _i;
    int _lifetime;
    void _initEntity() {
        getQuad()->scale.v = glm::vec3(24.0f, 24.0f, 1.0f);
        getQuad()->pos.v = getQuad()->pos.v + glm::vec3(0.0f, 0.0f, 1.0f);
        getAnimState().setAnimState(rand() % 2);

        _i = 0;
        _lifetime = 16;
    }

    void _baseEntity() {
        _i++;
        if (_i >= _lifetime)
            kill();
        enqueue();
    }

    void _killEntity() {
        removeQuad();
    }
public:
    Effect1() : Entity() {}
};

class Effect2 : public Entity {
    int _i;
    int _lifetime;
    void _initEntity() {
        getQuad()->scale.v = glm::vec3(20.0f, 20.0f, 1.0f);
        getQuad()->pos.v = getQuad()->pos.v + glm::vec3(0.0f, 0.0f, 1.0f);
        getAnimState().setAnimState(rand() % 2);

        _i = 0;
        _lifetime = 16;
    }

    void _baseEntity() {
        _i++;
        if (_i >= _lifetime)
            kill();
        enqueue();
    }

    void _killEntity() {
        removeQuad();
    }
public:
    Effect2() : Entity() {}
};

class Effect3 : public Entity {
    int _i;
    int _lifetime;
    void _initEntity() {
        getQuad()->scale.v = glm::vec3(24.0f, 24.0f, 1.0f);
        getQuad()->pos.v = getQuad()->pos.v + glm::vec3(0.0f, 0.0f, 1.0f);
        getAnimState().setAnimState(rand() % 2);

        _i = 0;
        _lifetime = 20;
    }

    void _baseEntity() {
        _i++;
        if (_i >= _lifetime)
            kill();
        enqueue();
    }

    void _killEntity() {
        removeQuad();
    }
public:
    Effect3() : Entity() {}
};

class Gravity : public Script {
    glm::vec3 _accl;
    ObjectManager *_manager;
    bool _ready;

    void _init() {}
    void _base() {
        if (_ready) {
            int maxid = _manager->getMaxID();
            Object *obj;
            for (int i = 0; i < maxid; i++)
                if ((obj = _manager->getObject(i)))
                    obj->getBox()->vel = obj->getBox()->vel + _accl;
        }
        enqueue();
    }
    void _kill() {}

public:
    Gravity() : Script(), _accl(glm::vec3(0.0f)), _manager(nullptr), _ready(false) {}

    void gravitySetup(glm::vec3 accl, ObjectManager *manager) {
        _accl = accl;
        _manager = manager;
        _ready = true;
    }
};

Object *block1allocator() { return new Block1; }
Object *test1allocator() { return new Test1; }
Object *test2allocator() { return new Test2; }
Entity *effect1allocator() { return new Effect1; }
Entity *effect2allocator() { return new Effect2; }
Entity *effect3allocator() { return new Effect3; }

void loop(GLFWwindow *winhandle) {
    srand(time(NULL));
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv obj_glenv{256};

    std::cout << "Setting up texture array" << std::endl;
    obj_glenv.setTexArray(96, 136, 2);
    std::cout << "Setting texture" << std::endl;
    obj_glenv.setTexture(Image("objects.png"), 0, 0, 0);
    obj_glenv.setTexture(Image("effects.png"), 0, 0, 1);

    int pixelwidth = 256;
    int pixelheight = 256;
    int pixellayers = 16;

    float halfwidth = float(pixelwidth) / 2.0f;
    float halfheight = float(pixelheight) / 2.0f;

    std::cout << "Setting up view and projection matrices" << std::endl;
    obj_glenv.setView(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    obj_glenv.setProj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(pixellayers)));

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // initialize animation map
    std::cout << "Setting up animation map" << std::endl;
    std::unordered_map<std::string, Animation> animations = loadAnimations("./animconfig");

    // initialize filter map
    std::cout << "Setting up filter map" << std::endl;
    std::unordered_map<std::string, Filter> filters = loadFilters("./filterconfig");

    // initialize PhysEnv instance
    std::cout << "Setting up physenv" << std::endl;
    PhysEnv obj_physenv{256};

    // set up Executor instance
    std::cout << "Setting up executors" << std::endl;
    Executor gbl_executor{256};
    Executor obj_executor{256};

    // set up manager
    std::cout << "Setting up manager" << std::endl;
    ObjectManager obj_manager{256};
    obj_manager.setExecutor(&obj_executor);
    obj_manager.setGLEnv(&obj_glenv);
    obj_manager.setAnimations(&animations);
    obj_manager.setPhysEnv(&obj_physenv);
    obj_manager.setFilters(&filters);

    // add objects to manager
    std::cout << "Adding entities and objects to manager" << std::endl;
    obj_manager.addObject(block1allocator, "Block1", T_BLOCK1, true, true, true, true, "Block1", true, "Block1");
    obj_manager.addObject(test1allocator, "Test1", T_TEST1, true, true, true, true, "Test1", true, "Test1");
    obj_manager.addObject(test2allocator, "Test2", T_TEST2, true, true, true, true, "Test2", true, "Test2");
    obj_manager.addEntity(effect1allocator, "Effect1", T_EFFECT1, true, true, true, true, "Effect1");
    obj_manager.addEntity(effect2allocator, "Effect2", T_EFFECT2, true, true, true, true, "Effect2");  
    obj_manager.addEntity(effect3allocator, "Effect3", T_EFFECT3, true, true, true, true, "Effect3");

    // set up instances
    std::cout << "Setting up gravity" << std::endl;
    Gravity gravity;
    gravity.gravitySetup(glm::vec3(0.0f, -0.03f, 0.0f), &obj_manager);
    gravity.scriptSetup(&gbl_executor);
    gravity.enqueue();

    // spawn object
    std::cout << "Spawning block" << std::endl;
    Object *block1 = obj_manager.getObject(obj_manager.spawnObject("Block1"));
    block1->getBox()->pos = glm::vec3(0.0f, -64.0f, 0.0f);
    
    // start loop
    std::cout << "Starting loop" << std::endl;
    int i = 0;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // previously 15
        if (i % 15 == 0) {
            Entity *effect3 = obj_manager.getEntity(obj_manager.spawnEntity("Effect3"));
            Object *test1 = obj_manager.getObject(obj_manager.spawnObject("Test1"));

            effect3->getQuad()->pos.v = glm::vec3(0.0f, 64.0f, 0.0f);
            test1->getBox()->pos = glm::vec3(0.0f, 64.0f, 0.0f);
            test1->getBox()->vel = random_angle(glm::vec3(0.0f, 1.25f, 0.0f), 15.0f);

            //test1->setPhysVel(glm::vec3(0.0f, -15.0f, 0.0f));
        }

        // run global scripts
        gbl_executor.runExec();
        gbl_executor.runKill();

        // run object scripts
        obj_physenv.detectCollision();
        obj_executor.runExec();
        obj_executor.runKill();

        // update graphic environment and draw
        obj_glenv.draw();

        glfwSwapBuffers(winhandle);

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i++;
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}