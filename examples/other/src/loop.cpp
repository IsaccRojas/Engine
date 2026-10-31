#include "loop.hpp"

void loop() {
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(globalstate.glfwstate.getWindowHandle()) && !globalstate.input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        checkState();
        switch (globalstate.game_state) {
            case 0:
                runState_Start();
                break;
            case 1:
                runState_StageContinue();
                break;
            case 2:
                runState_StageComplete();
                break;
            default:
                throw std::runtime_error("Invalid state in loop()");
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

void checkState() {
    switch (globalstate.game_state) {
        case 0:
            // transition to 1
            if (globalstate.input.get_space()) {
                if (!globalstate.player) {
                    globalstate.player = globalstate.executor.spawnEntity("Entity_Player", Transform{});
                    globalstate.player->attachNullableRef(&globalstate.player);
                }
                globalstate.progress = 0.0f;
                globalstate.game_state = 1;
            }
            break;

        case 1:
            // transition to 0
            if (!globalstate.player) {
                globalstate.game_state = 0;
                break;
            }

            // transition to 2
            if (globalstate.progress >= 100.0f) {
                globalstate.game_state = 2;
                break;
            }
            break;

        case 2:
            // transition to 1
            globalstate.progress = 0.0f;
            globalstate.game_state = 1;
            break;
        
        default:
            throw std::runtime_error("Invalid state in checkState()");
    }
}

void runState_Start() {

}

void runState_StageContinue() {
    if (globalstate.spawn_cooldown > 0)
        globalstate.spawn_cooldown--;
    
    if (globalstate.spawn_cooldown == 0) {
        globalstate.executor.spawnEntity("Entity_Enemy_Chaser", Transform{glm::vec3(32.0f + (rand() % 64), (rand() % 128) - 64, 0.0f), glm::vec3(1.0f)});
        globalstate.spawn_cooldown = 90 + (rand() % 90);
    }

    globalstate.progress += 0.05f;
}

void runState_StageComplete() {

}