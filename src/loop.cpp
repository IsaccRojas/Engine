#include "loop.hpp"

class Test1 : public Object {
    int _i;
    bool _colliding;

    void _initObject() {
        std::cout << "Test1 instance initialized" << std::endl;

        genQuad(glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 1.0f));
        enableCollision();

        _i = 0;
    };

    void _baseObject() {
        setPhysPos(glm::vec3(0.0f, glm::sin(double(_i) / 64.0f) * 32.0f, 0.0f));
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
    Test1() : Object(glm::vec3(16.0f, 16.0f, 0.0f)) {}
};

Object *test1allocator() { return new Test1; }

void loop(GLFWwindow *winhandle) {
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv glenv{256};

    std::cout << "Setting up texture array" << std::endl;
    glenv.settexarray(32, 32, 1);
    std::cout << "Setting texture" << std::endl;
    glenv.settexture(Image("objects.png"), 0, 0, 0);

    int pixelwidth = 256;
    int pixelheight = 256;
    int pixellayers = 16;

    float halfwidth = float(pixelwidth) / 2.0f;
    float halfheight = float(pixelheight) / 2.0f;

    std::cout << "Setting up view and projection matrices" << std::endl;
    glenv.setview(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    glenv.setproj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(pixellayers)));
    

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // initialize animation map
    std::cout << "Setting up animation map" << std::endl;
    std::unordered_map<std::string, Animation> animations = loadAnimations(".");

    // set up Executor instance
    std::cout << "Setting up executor" << std::endl;
    Executor executor{256};

    // set up Collider instance
    std::cout << "Setting up collider" << std::endl;
    Collider collider{256};

    // set up manager
    std::cout << "Setting up manager" << std::endl;
    Manager manager{256};
    manager.setExecutor(&executor);
    manager.setGLEnv(&glenv);
    manager.setAnimations(&animations);
    manager.setCollider(&collider);

    // add objects to manager
    std::cout << "Adding objects to manager" << std::endl;
    manager.addObject(test1allocator, "Test1", true, true, true, "Test1", true);

    // set up test entities
    std::cout << "Setting up test object" << std::endl;
    manager.spawnObject("Test1");

    // start loop
    std::cout << "Starting loop" << std::endl;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // run execution environment
        collider.collide();
        executor.runExec();
        executor.runKill();

        // update graphic environment and draw
        glenv.draw();

        glfwSwapBuffers(winhandle);
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}