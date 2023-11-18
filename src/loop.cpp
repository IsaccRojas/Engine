#include "loop.hpp"

class Test1 : public Object {
    int _i;
    bool _colliding;

    void _initObject() {
        std::cout << "Test1 instance initialized" << std::endl;

        genQuad(glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 1.0f));
        
        setPhysDim(glm::vec3(16.0f, 16.0f, 0.0f));
        enableCollision();

        setPhysVel(glm::vec3(0.2f, 1.25f, 0.0f));
        _i = 0;
    };

    void _baseObject() {
        //setPhysPos(glm::vec3(0.0f, glm::sin(double(_i) / 64.0f) * 32.0f, 0.0f));
        _i++;
        
        setVisPos(getPhysPos());
        enqueue();
    };

    void _killObject() {
        std::cout << "Test1 instance being killed" << std::endl;

        eraseQuad();
        disableCollision();
    };

    void _collisionObject(int x) {
        _colliding = true;
    }

public:
    Test1() : Object() {}
};

class Gravity : public Script {
    glm::vec3 _accl;
    Manager *_manager;
    bool _ready;

    void _init() {}
    void _base() {
        if (_ready) {
            int maxid = _manager->getMaxID();
            Object *obj;
            for (int i = 0; i < maxid; i++)
                if ((obj = _manager->getObject(i)))
                    obj->setPhysVel(obj->getPhysVel() + _accl);
        }
        enqueue();
    }
    void _kill() {}

public:
    Gravity() : Script(), _accl(glm::vec3(0.0f)), _manager(nullptr), _ready(false) {}

    void gravitySetup(glm::vec3 accl, Manager *manager) {
        _accl = accl;
        _manager = manager;
        _ready = true;
    }
};

Object *test1allocator() { return new Test1; }

void loop(GLFWwindow *winhandle) {
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv obj_glenv{256};

    std::cout << "Setting up texture array" << std::endl;
    obj_glenv.settexarray(32, 32, 1);
    std::cout << "Setting texture" << std::endl;
    obj_glenv.settexture(Image("objects.png"), 0, 0, 0);

    int pixelwidth = 256;
    int pixelheight = 256;
    int pixellayers = 16;

    float halfwidth = float(pixelwidth) / 2.0f;
    float halfheight = float(pixelheight) / 2.0f;

    std::cout << "Setting up view and projection matrices" << std::endl;
    obj_glenv.setview(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    obj_glenv.setproj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(pixellayers)));
    

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // initialize animation map
    std::cout << "Setting up animation map" << std::endl;
    std::unordered_map<std::string, Animation> animations = loadAnimations(".");

    // set up Executor instance
    std::cout << "Setting up executors" << std::endl;
    Executor gbl_executor{256};
    Executor obj_executor{256};

    // set up Collider instance
    std::cout << "Setting up collider" << std::endl;
    Collider obj_collider{256};

    // set up manager
    std::cout << "Setting up manager" << std::endl;
    Manager obj_manager{256};
    obj_manager.setExecutor(&obj_executor);
    obj_manager.setGLEnv(&obj_glenv);
    obj_manager.setAnimations(&animations);
    obj_manager.setCollider(&obj_collider);

    // add objects to manager
    std::cout << "Adding object to manager" << std::endl;
    obj_manager.addObject(test1allocator, "Test1", true, true, true, "Test1", true);

    // set up instances
    std::cout << "Setting up gravity" << std::endl;
    Gravity gravity;
    gravity.gravitySetup(glm::vec3(0.0f, -0.03f, 0.0f), &obj_manager);
    gravity.scriptSetup(&gbl_executor);
    gravity.enqueue();

    // spawn object
    std::cout << "Spawning test object" << std::endl;
    obj_manager.spawnObject("Test1");
    
    // start loop
    std::cout << "Starting loop" << std::endl;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // run global scripts
        gbl_executor.runExec();
        gbl_executor.runKill();

        // run object scripts
        obj_collider.collide();
        obj_executor.runExec();
        obj_executor.runKill();

        // update graphic environment and draw
        obj_glenv.draw();

        glfwSwapBuffers(winhandle);
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}