#include "loop.hpp"

// ES_Player allocator that holds reference to input state
class ES_PlayerAllocator : public EntityScriptAllocatorInterface {
    GLFWInput *_input_state;
    EntityScript *_allocate() override { return new ES_Player(_input_state); }
public:
    ES_PlayerAllocator(GLFWInput *input_state) : _input_state(input_state) {}
};

void loop(CoreResources *core) {
    std::cout << "Setting up loop" << std::endl;
    
    srand(time(NULL));

    ES_PlayerAllocator alloc_ES_Player(&(core->input));
    core->entityexecutor.addEntityScript(&alloc_ES_Player, "ES_Player", nullptr, nullptr);
    
    core->entitymanager.addEntity(
        EntityInfo{
            {"ES_Player", 0},
            {{glm::vec3(0.0f), glm::vec3(0.0f), glm::vec4(1.0f), GLE_RECT, "Player", glm::vec3(0.0f), glm::vec2(0.0f), 0.0f}},
            {{Transform(), glm::vec3(0.0f), nullptr, "Player"}},
            "Group_Player"
        },
        "Player"
    );
    Entity *player = core->entitymanager.spawnEntity("Player");
    player->attributes3f()["pos"] = glm::vec3(0.0f, 32.0f, 0.0f);
    
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->physspace_box.step();

        core->input.update();

        core->entityexecutor.runExecQueue(0);
        core->entityexecutor.runSpawnQueue();
        core->entityexecutor.runKillQueue();
        core->entityexecutor.runUpdate();

        core->entitymanager.checkEntities();

        core->glenv.update();
        core->glenv.drawQuads();
        glfwSwapBuffers(core->state.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}