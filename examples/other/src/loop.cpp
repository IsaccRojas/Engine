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
    GenericEntityScriptAllocator<ES_Chaser> alloc_ES_Chaser;

    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Player"]}, "Quad_Player");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_PurpleSquare"]}, "Quad_PurpleSquare");

    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Player"]}, "EntityCollider_Player");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Enemy"]}, "EntityCollider_Enemy");

    core->entityexecutor.addEntityScript(EntityScriptInfo{&alloc_ES_Player, nullptr, nullptr}, "ES_Player");
    core->entityexecutor.addEntityScript(EntityScriptInfo{&alloc_ES_Chaser, nullptr, nullptr}, "ES_Chaser");
    
    core->entitymanager.addEntity(EntityInfo{"Group_Player", "ES_Player", 0, true, {"Quad_Player"}, {"EntityCollider_Player"}}, "Entity_Player");
    core->entitymanager.addEntity(EntityInfo{"Group_Enemy", "ES_Chaser", 0, true, {"Quad_PurpleSquare"}, {"EntityCollider_Enemy"}}, "Entity_Dummy");
    
    Entity *player = core->entitymanager.spawnEntity("Entity_Player", Transform{glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(1.0f)});
    Entity *dummy = core->entitymanager.spawnEntity("Entity_Dummy",Transform{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->collisionspace.detectCollisionAABB();

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