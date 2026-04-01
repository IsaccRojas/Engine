#include "loop.hpp"

void loop(CoreResources *core) {
    std::cout << "Setting up loop" << std::endl;
    
    srand(time(NULL));

    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Player"]}, "Quad_Player");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_BasicEnemy"]}, "Quad_BasicEnemy");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(32.0f, 32.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Slash"]}, "Quad_Slash");

    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Player"]}, "EntityCollider_Player");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), core->filters["Filter_Player"]}, "EntityCollider_Hitbox");

    core->executor.addEntityScript(EntityScriptInfo{&core->provider_ES_Player, nullptr, nullptr}, "ES_Player");
    core->executor.addEntityScript(EntityScriptInfo{&core->globalresources.provider_ES_Chaser, nullptr, nullptr}, "ES_Chaser");
    core->executor.addEntityScript(EntityScriptInfo{&core->globalresources.provider_ES_Lifetime, nullptr, nullptr}, "ES_Lifetime");
    
    core->entitymanager.addEntity(EntityInfo{"Group_Player", "ES_Player", 0, true, {"Quad_Player"}, {"EntityCollider_Player"}}, "Entity_Player");
    core->entitymanager.addEntity(EntityInfo{"Group_Enemy", "ES_Chaser", 0, true, {"Quad_BasicEnemy"}, {"EntityCollider_Enemy"}}, "Entity_BasicEnemy");
    core->entitymanager.addEntity(EntityInfo{"Group_Hitbox", "ES_Lifetime", 0, true, {}, {"EntityCollider_Hitbox"}}, "Entity_Hitbox");
    core->entitymanager.addEntity(EntityInfo{"Group_Effect", "ES_Lifetime", 0, true, {"Quad_Slash"}, {}}, "Entity_Slash");
    
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