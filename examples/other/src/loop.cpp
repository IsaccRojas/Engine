#include "loop.hpp"

void loop(CoreResources *core) {
    std::cout << "Setting up loop" << std::endl;
    
    core->entitymanager.spawnEntity("Entity_Player", Transform{glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(1.0f)});
    core->entitymanager.spawnEntity("Entity_BasicEnemy",Transform{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->glfwstate.getWindowHandle()) && !core->glfwinput.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->collisionspace.detectCollisionAABB();

        core->glfwinput.update();

        core->entityscriptexecutor.runExecQueue(0);
        core->entityscriptexecutor.runSpawnQueue();
        core->entityscriptexecutor.runKillQueue();
        core->entityscriptexecutor.runUpdate();
        checkTileCollision(core);

        core->entitymanager.checkEntities();

        core->glenv.update();
        core->glenv.drawQuads();

        glfwSwapBuffers(core->glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}

void checkTileCollision(CoreResources *core) {

}