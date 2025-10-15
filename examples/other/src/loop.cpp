#include "loop.hpp"

// Player allocator that holds reference to input state
class PlayerAllocator : public EntityAllocatorInterface {
    GLFWInput *_input_state;
    Entity *_allocate(int tag) override { return new Player("Player", "Player", _input_state); }
public:
    PlayerAllocator(GLFWInput *input_state) : _input_state(input_state) {}
};

void loop(CoreResources *core) {
    srand(time(NULL));

    PlayerAllocator alloc_Player(&(core->input));
    core->executor.addEntity(&alloc_Player, "Player", 0, true, nullptr, nullptr);

    //core->glenv.genQuad(glm::vec3(0.0f), glm::vec3(16.0f), glm::vec4(1.0f), 0.0f, glm::vec3(160.0f, 0.0f, 0.0f), glm::vec2(16.0f, 16.0f), GLE_RECT);
    core->executor.enqueueSpawnEntity("Player", 0, -1, Transform{});

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->input.update();

        core->glenv.update();
        core->glenv.drawQuads();

        core->box_space.step();

        core->executor.runExecQueue(0);
        core->executor.runSpawnQueue();
        core->executor.runKillQueue();

        glfwSwapBuffers(core->state.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}