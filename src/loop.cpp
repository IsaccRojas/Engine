#include "loop.hpp"

class Test : public Entity {
    int i;
    int lifetime;

    void _initEntity() {
        std::cout << "initialized" << std::endl;
        i = 0;
        lifetime = 180;
    };

    void _baseEntity() {
        i++;
        if (i == lifetime)
            owner()->queueErase(id());
    };

    void _killEntity() {
        std::cout << "being killed" << std::endl;
    };

public:
    Test() {}    
};

Entity *testallocator() { return new Test; }

void loop(GLFWwindow *winhandle) {
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv glenv{256};
    glenv.settexarray(8, 8, 1);
    glenv.settexture(Image("texture.png"), 0, 0, 0);
    glenv.setview(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    glenv.setproj(glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 16.0f));

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // set up ExecEnv instance
    std::cout << "Setting up execenv" << std::endl;
    ExecEnv execenv{256};

    // set up animation map
    std::cout << "Setting up animations" << std::endl;
    auto animations = loadAnimations(".");

    // set up spawner
    std::cout << "Setting up spawner" << std::endl;
    EntitySpawner spawner(&glenv);
    spawner.loadAnimations(".");
    spawner.add(testallocator, "Test", "Test");

    // set up test entity
    std::cout << "Setting up test entity" << std::endl;
    Entity* test = spawner.spawn("Test");
    execenv.push(test);

    // start loop
    std::cout << "Starting loop" << std::endl;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // run execution environment
        execenv.queueExecAll();
        execenv.runExec();
        execenv.runErase();

        // update graphic environment and draw
        glenv.draw();

        glfwSwapBuffers(winhandle);
    };
    
    // terminate GLFW
    std::cout << "Terminating" << std::endl;
    delete test;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}