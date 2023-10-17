#include "loop.hpp"

void loop(GLFWwindow *winhandle) {
    /*
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv glenv{16};
    glenv.settexarray(8, 8, 1);
    glenv.settexture(Image("texture.png"), 0, 0, 0);
    glenv.setview(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    glenv.setproj(glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.0f, 16.0f));

    // generate some initial quads
    std::cout << "Generating initial quads" << std::endl;
    float tcoord;
    for (int i = 0; i < 10; i++) {
        tcoord = (i % 2) ? 2.0f : 0.0f;
        glenv.genQuad(
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(tcoord, tcoord, 0.0f),
            glm::vec2(3.0f, 3.0f)
        );
    }

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // set up loop
    std::cout << "Setting up loop" << std::endl;
    int i = 0;
    float offset = 0.0f;
    std::chrono::duration<float> diff;
    std::chrono::milliseconds diffmilli;
    auto prevtime = std::chrono::system_clock::now();
    std::vector<int> quadids = glenv.getids();
    Quad *quad;
    int spawnmode = 0;
    srand(time(NULL));
    */

    ExecEnv execenv(256);

    // --- loop setup ---

    //start loop
    std::cout << "Starting loop" << std::endl;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();

        /*
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        //decide whether to spawn or erase
        if (spawnmode == 0) {
            if (quadids.size() <= 1)
                spawnmode = 1;
        }
        else {
            if (quadids.size() >= 10)
                spawnmode = 0;
        }

        //get time and spawn or delete a random quad every second
        diff = std::chrono::system_clock::now() - prevtime;
        diffmilli = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        if (diffmilli.count() >= 250) {
            prevtime = std::chrono::system_clock::now();
            
            //spawn or delete a random quad
            if (spawnmode)
                glenv.genQuad(
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3(rand() % 2 + 1, rand() % 2 + 1, 1.0f),
                    glm::vec3(rand() % 5, rand() % 5, 0.0f),
                    glm::vec2(3.0f, 3.0f)
                );
            else
                glenv.erase(quadids[rand() % quadids.size()]);

            quadids = glenv.getids();
        }

        //draw quads in a circle, with an offset based on the number of quads
        for (int j = 0; j < quadids.size(); j++) {
            offset = (float(j) / float(quadids.size())) * PI * 2.0f;
            quad = glenv.get(quadids[j]);
            quad->pos.v.x = glm::cos((float(i) / 64.0f) + offset);
            quad->pos.v.y = glm::sin((float(i) / 64.0f) + offset);
        }

        glenv.update();
        glenv.draw();

        glfwSwapBuffers(winhandle);
        i++;
        */
    };
    
    // terminate GLFW
    std::cout << "Terminating" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}