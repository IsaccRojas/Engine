#include "loop.hpp"

void loop() {
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
        globalstate.executor.runUpdate();

        globalstate.manager.update();

        globalstate.glenv.update();
        globalstate.glenv.drawQuads();

        glfwSwapBuffers(globalstate.glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}