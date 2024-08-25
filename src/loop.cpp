#include "loop.hpp"

enum Type { T_BASIC_ORBSHOT, T_EFFECT_BIGSMOKE, T_EFFECT_ORBSHOTPARTICLE, T_EFFECT_ORBSHOTBOOM };


class OrbShot : public Basic {
    int _i;
    int _lifetime;

    void _initBasic() {
        getBox()->mass = 0.0f;
        getQuad()->scale.v = glm::vec3(6.0f, 6.0f, 1.0f);

        _i = 0;
        _lifetime = 119;
        getAnimState().setAnimState(0);
    }

    void _baseBasic() {
        _i++;

        if (_i % 10 == 0) {
            int id = getManager()->spawnEntity("OrbShotParticle");
            if (id >= 0) {
                Entity *particle = getManager()->getEntity(id);
                particle->getQuad()->pos.v = getBox()->pos;
            }
        }

        if (_i >= _lifetime)
            kill();
    }

    void _killBasic() {
        int id = getManager()->spawnEntity("OrbShotBoom");
        if (id >= 0) {
            Entity *boom = getManager()->getEntity(id);
            boom->getQuad()->pos.v = getBox()->pos;
        }
    }

    void _collisionBasic(Box *box) {
        kill();
    }

public:
    OrbShot() : Basic(glm::vec3(6.0f, 6.0f, 0.0f)) {}
};

class BigBoom : public Effect {
    void _initEffect() {
        getAnimState().setAnimState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    BigBoom() : Effect(glm::vec3(24.0f, 24.0f, 1.0f), 16) {}
};

class SmallBoom : public Effect {
    void _initEffect() {
        getAnimState().setAnimState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    SmallBoom() : Effect(glm::vec3(20.0f, 20.0f, 1.0f), 16) {}
};

class BigSmoke : public Effect {
    void _initEffect() {
        getAnimState().setAnimState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    BigSmoke() : Effect(glm::vec3(24.0f, 24.0f, 1.0f), 20) {}
};

class OrbShotParticle : public Effect {
    void _initEffect() { getQuad()->pos.v.z = -1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotParticle() : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 24) {}
};

class OrbShotBoom : public Effect {
    void _initEffect() { getQuad()->pos.v.z = -1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotBoom() : Effect(glm::vec3(8.0f, 8.0f, 1.0f), 24) {}
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

Object *OrbShot_allocator() { return new OrbShot; }
Entity *BigBoom_allocator() { return new BigBoom; }
Entity *SmallBoom_allocator() { return new SmallBoom; }
Entity *BigSmoke_allocator() { return new BigSmoke; }
Entity *OrbShotParticle_allocator() { return new OrbShotParticle; }
Entity *OrbShotBoom_allocator() { return new OrbShotBoom; }

void loop(GLFWwindow *winhandle) {
    srand(time(NULL));
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv obj_glenv{256};

    std::cout << "Setting up texture array" << std::endl;
    obj_glenv.setTexArray(96, 62, 2);
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
    obj_manager.addObject(OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, true, true, true, "OrbShot", true, "ProjectileFriendly");
    obj_manager.addEntity(BigSmoke_allocator, "BigSmoke", T_EFFECT_BIGSMOKE, true, true, true, true, "BigSmoke");
    obj_manager.addEntity(OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, true, true, true, "OrbShotParticle");
    obj_manager.addEntity(OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, true, true, true, "OrbShotBoom");

    // set up instances
    std::cout << "Setting up gravity" << std::endl;
    Gravity gravity;
    gravity.gravitySetup(glm::vec3(0.0f, -0.03f, 0.0f), &obj_manager);
    gravity.scriptSetup(&gbl_executor);
    gravity.enqueue();

    // set up input
    std::cout << "Setting up input" << std::endl;
    Input input(winhandle, pixelwidth, pixelheight);

    // set up player
    std::cout << "Setting up player" << std::endl;
    Player player;
    player.scriptSetup(&obj_executor);
    player.entitySetup(&obj_glenv, &animations["Player"]);
    player.objectSetup(&obj_physenv, &filters["Player"]);
    player.playerSetup(&input, &obj_manager);
    player.enqueue();
    
    // start loop
    std::cout << "Starting loop" << std::endl;
    int i = 0;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*
        if (i % 15 == 0 && false) {
            Entity *effect3 = obj_manager.getEntity(obj_manager.spawnEntity("BigSmoke"));
            Object *test1 = obj_manager.getObject(obj_manager.spawnObject("BigBomb"));

            effect3->getQuad()->pos.v = glm::vec3(0.0f, 64.0f, 0.0f);
            test1->getBox()->pos = glm::vec3(0.0f, 64.0f, 0.0f);
            test1->getBox()->vel = random_angle(glm::vec3(0.0f, 1.25f, 0.0f), 15.0f);

            //test1->setPhysVel(glm::vec3(0.0f, -15.0f, 0.0f));
        }
        */
        
        input.update();

        // run global scripts
        //gbl_executor.runExec();
        //gbl_executor.runKill();

        // run object scripts
        obj_executor.runExec();
        obj_executor.runKill();

        // update physics environment and detect collisions
        obj_physenv.step();
        obj_physenv.detectCollision();

        // update graphic environment and draw
        obj_glenv.update();
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