#include "loop.hpp"

class Square : public Script {
    GLEnv *_glenv;
    Quad *_quad;
    int _quadid;
    int _i;

    void _init() {
        std::cout << "Square created" << std::endl;
    }
    void _base() { 
        // update quad's y coordinate according to sine function
        _quad->pos.v.y = glm::sin(float(_i) / 64.0f);
        _i++;

        // erase self after 5 executions
        if (_i >= 180)
            this->owner()->queueErase(this->id());
    }
    void _kill() {
        std::cout << "Square killed" << std::endl;

        // erase own quad
        _glenv->erase(_quadid);
    }

public:
    Square(GLEnv *glenv) : Script(), _glenv(glenv), _i(0) {
        _quadid = _glenv->genQuad(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        );
        _quad = _glenv->get(_quadid);
    }
    ~Square() {}
};

Script *Square_allocator(GLEnv *glenv) {
    return new Square(glenv);
}

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
    ExecEnv execenv{256};

    // bind GLEnv instance to allocator and add allocator to ExecEnv
    execenv.add(std::bind(Square_allocator, &glenv), "Square");

    // start loop
    std::cout << "Starting loop" << std::endl;
    int i = 0;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (i == 0)
            execenv.push("Square");

        // run execution environment
        execenv.queueExecAll();
        execenv.runExec();
        execenv.runErase();

        // update graphic environment and draw
        glenv.update();
        glenv.draw();

        glfwSwapBuffers(winhandle);
        i++;
    };
    
    // terminate GLFW
    std::cout << "Terminating" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}