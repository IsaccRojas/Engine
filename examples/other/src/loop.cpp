#include "loop.hpp"

void loop(CoreResources *core) {
    std::cout << "Setting up loop" << std::endl;
    
    srand(time(NULL));
    
    core->entitymanager.spawnEntity("Entity_Player", Transform{glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(1.0f)});
    core->entitymanager.spawnEntity("Entity_BasicEnemy",Transform{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->collisionspace.detectCollisionAABB();

        core->input.update();

        core->executor.runExecQueue(0);
        core->executor.runSpawnQueue();
        core->executor.runKillQueue();
        core->executor.runUpdate();

        core->entitymanager.checkEntities();

        core->glenv.update();
        core->glenv.drawQuads();

        glfwSwapBuffers(core->state.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}