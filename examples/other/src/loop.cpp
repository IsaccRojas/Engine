#include "loop.hpp"

void loop(CoreResources* core) {
    genLevel(core);
    
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->glfwstate.getWindowHandle()) && !core->glfwinput.get_esc()) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->collisionspace.detectCollisionAABB();

        core->glfwinput.update();

        core->entityscriptexecutor.runExecQueue(0);
        core->entityscriptexecutor.runExecQueue(1);
        core->entityscriptexecutor.runSpawnQueue();
        core->entityscriptexecutor.runKillQueue();
        core->entityscriptexecutor.runUpdate();

        core->entitymanager.checkEntities();

        core->glenv.update();
        core->glenv.drawQuads();

        glfwSwapBuffers(core->glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}

void genLevel(CoreResources* core) {
    auto& m = core->globalresources.map;
    auto& mi = core->globalresources.mapinfo;

    // initialize map
    for (unsigned x = 0; x < mi.coord_dimensions.x; x++) {
        for (unsigned y = 0; y < mi.coord_dimensions.y; y++) {
            TileInfo& tile = m[x][y];

            // solid if on edge or both coordinates are even
            if (x == 0 || x == mi.coord_dimensions.x - 1 || y == 0 || y == mi.coord_dimensions.y - 1 || (isEven(x) && isEven(y)))
                tile.value = 1;
            else
                tile.value = 0;
            
            if (
                (x == 2 && y == 2) ||
                (x == 4 && y == 2) ||
                (x == 2 && y == 4) ||
                (x == 4 && y == 4) ||
                (x == 6 && y == 4) ||
                (x == 6 && y == 6) ||
                (x == 6 && y == 8) ||
                (x == 8 && y == 4) ||
                (x == 8 && y == 6) ||
                (x == 8 && y == 8)
            )
                tile.value = 0;
             
            // create graphic
            if (tile.value > 0)
                tile.quad_id = core->glenv.genQuad(
                    "Quad_SolidTile",
                    Transform{glm::vec3(
                        ((x * mi.unit_pixel_dimensions.x) + (mi.unit_pixel_dimensions.x / 2.0f)) + mi.coord_origin.x,
                        ((y * mi.unit_pixel_dimensions.y) + (mi.unit_pixel_dimensions.y / 2.0f)) + mi.coord_origin.y, 
                        -1.0f
                    ), glm::vec3(1.0f)}
                );
        }
    }

    core->entitymanager.spawnEntity("Entity_Player", Transform{glm::vec3(128.0f, 128.0f, 0.0f), glm::vec3(1.0f)});
    //core->entitymanager.spawnEntity("Entity_BasicEnemy",Transform{glm::vec3(32.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

    core->entitymanager.spawnEntity("Entity_Key", Transform{toVec3(core->globalresources.mapinfo.toPixels(glm::ivec2(1, 1)), 0.0f), glm::vec3(1.0f)});
    core->globalresources.container_Pickup.getLastInstance()->item_name = "key";

    core->entitymanager.spawnEntity("Entity_Stairs", Transform{toVec3(core->globalresources.mapinfo.toPixels(glm::ivec2(1, 10)), 0.0f), glm::vec3(1.0f)});
}