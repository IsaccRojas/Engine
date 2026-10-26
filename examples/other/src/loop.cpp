#include "loop.hpp"

void loop() {
    globalstate.executor.spawnEntity("Entity_Player", Transform{});
    globalstate.executor.spawnEntity("Entity_Enemy_Chaser", Transform{glm::vec3(96.0f, 96.0f, 0.0f), glm::vec3(1.0f)});

    int cooldown = 120;
    
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(globalstate.glfwstate.getWindowHandle()) && !globalstate.input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (cooldown > 0)
            cooldown--;
        
        if (cooldown == 0) {
            globalstate.executor.spawnEntity("Entity_Enemy_Chaser", Transform{glm::vec3(32.0f + (rand() % 64), (rand() % 128) - 64, 0.0f), glm::vec3(1.0f)});
            cooldown = 90 + (rand() % 90);
        }
        

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