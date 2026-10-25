#include "loop.hpp"

void loop() {
    globalstate.executor.spawnEntity("Entity_Player", Transform{});
    globalstate.executor.spawnEntity("Entity_Chaser", Transform{glm::vec3(96.0f, 96.0f, 0.0f), glm::vec3(1.0f)});
    
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(globalstate.glfwstate.getWindowHandle()) && !globalstate.input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        globalstate.collisionspace.detectCollisionAABB();

        globalstate.input.update();

        globalstate.executor.runExecQueue(0);
        globalstate.executor.runExecQueue(1);
        globalstate.executor.runSpawnQueue();
        globalstate.executor.runKillQueue();
        globalstate.executor.update();

        globalstate.glenv.update();
        globalstate.glenv.drawQuads();

        glfwSwapBuffers(globalstate.glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}