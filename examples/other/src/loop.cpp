#include "loop.hpp"

// ES_Player allocator that holds reference to input state
class PlayerProvider : public EntityScriptProviderInterface<ES_Player> {
    GLFWInput* _input_state;
    ES_Player* _providerAllocate() override { return new ES_Player(_input_state); }
public:
    PlayerProvider(GLFWInput *input_state) : _input_state(input_state) {}
};

void loop(CoreResources *core) {
    std::cout << "Setting up loop" << std::endl;
    
    srand(time(NULL));

    PlayerProvider alloc_ES_Player(&(core->input));
    GenericEntityScriptProvider<ES_Chaser> provider_ES_Chaser;
    GenericEntityScriptProvider<ES_Hitbox> provider_ES_Hitbox;

    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Player"]}, "Quad_Player");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_PurpleSquare"]}, "Quad_PurpleSquare");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_YellowSquare"]}, "Quad_YellowSquare");

    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Player"]}, "EntityCollider_Player");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), core->filters["Filter_Player"]}, "EntityCollider_Hitbox");

    core->entityexecutor.addEntityScript(EntityScriptInfo{&alloc_ES_Player, nullptr, nullptr}, "ES_Player");
    core->entityexecutor.addEntityScript(EntityScriptInfo{&provider_ES_Chaser, nullptr, nullptr}, "ES_Chaser");
    core->entityexecutor.addEntityScript(EntityScriptInfo{&provider_ES_Hitbox, nullptr, nullptr}, "ES_Hitbox");
    
    core->entitymanager.addEntity(EntityInfo{"Group_Player", "ES_Player", 0, true, {"Quad_Player"}, {"EntityCollider_Player"}}, "Entity_Player");
    core->entitymanager.addEntity(EntityInfo{"Group_Enemy", "ES_Chaser", 0, true, {"Quad_PurpleSquare"}, {"EntityCollider_Enemy"}}, "Entity_Dummy");
    core->entitymanager.addEntity(EntityInfo{"Group_Hitbox", "ES_Hitbox", 0, true, {"Quad_YellowSquare"}, {"EntityCollider_Hitbox"}}, "Entity_Hitbox");
    
    core->entitymanager.spawnEntity("Entity_Player", Transform{glm::vec3(0.0f, 32.0f, 0.0f), glm::vec3(1.0f)});
    core->entitymanager.spawnEntity("Entity_Dummy",Transform{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

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