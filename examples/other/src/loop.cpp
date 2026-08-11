#include "loop.hpp"

void loop(CoreResources* core) {
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->glfwstate.getWindowHandle()) && !core->glfwinput.get_esc()) {
        /*
        std::cout 
            << core->globalresources.level_clear_started
            << " "
            << core->globalresources.level_generated
            << " "
            << core->globalresources.stairs_entered
            << " ("
            << core->entitymanager.groupSize("Group_Spawnable")
            << ")"
            << std::endl;
        */
        
        // poll for entities if level clear started; else, initiate generation or clear as needed
        if (core->globalresources.level_clear_started) {
            if (!core->entitymanager.groupSize("Group_Spawnable")) {
                core->globalresources.level_generated = false;
                core->globalresources.level_clear_started = false;
            }

        } else {
            // generate level if none generated; else, check if level needs to be cleared
            if (!core->globalresources.level_generated) {
                genLevel(core);
                core->globalresources.level_generated = true;

            } else {
                if (core->globalresources.stairs_entered) {
                    clearLevel(core);
                    core->globalresources.stairs_entered = false;
                    core->globalresources.level_clear_started = true;
                }
            }
        }

        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        core->collisionspace.detectCollisionAABB();

        core->glfwinput.update();

        core->entityscriptexecutor.runExecQueue(0);
        core->entityscriptexecutor.runExecQueue(1);
        core->entityscriptexecutor.runSpawnQueue();
        core->entityscriptexecutor.runKillQueue();
        core->entityscriptexecutor.runUpdate();

        core->entitymanager.update();

        core->glenv.update();
        core->glenv.drawQuads();

        glfwSwapBuffers(core->glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}

void clearLevel(CoreResources* core) {
    // remove existing map and entities
    if (!core->globalresources.level_generated)
        throw std::runtime_error("Attempt to clear level when none is generated");

    // remove existing map and entities
    if (core->globalresources.level_clear_started)
        throw std::runtime_error("Attempt to clear level when clear is already in progress");
    
    auto& m = core->globalresources.map;
    auto& mi = core->globalresources.mapinfo;

    // remove quads and unset tile fields
    for (unsigned x = 0; x < mi.coord_dimensions.x; x++) {
        for (unsigned y = 0; y < mi.coord_dimensions.y; y++) {
            TileInfo& tile = m[x][y];

            if (tile.quad_id_lower >= 0)
                core->glenv.remove(tile.quad_id_lower);
            if (tile.quad_id_upper >= 0)
                core->glenv.remove(tile.quad_id_upper);

            tile.value = -1;
            tile.quad_id_lower = -1;
            tile.quad_id_upper = -1;
        }
    }

    // kill all entities
    for (auto iter = core->entitymanager.groupBegin("Group_Spawnable"); iter != core->entitymanager.groupEnd("Group_Spawnable"); iter++)
        (*iter)->kill();
}

void genLevel(CoreResources* core) {
    if (core->globalresources.level_generated)
        throw std::runtime_error("Attempt to generate level when it already exists");

    if (core->globalresources.level_clear_started)
        throw std::runtime_error("Attempt to generate level when level clearing is in progress");
    
    auto& m = core->globalresources.map;
    auto& mi = core->globalresources.mapinfo;

    // initialize map
    for (unsigned x = 0; x < mi.coord_dimensions.x; x++) {
        for (unsigned y = 0; y < mi.coord_dimensions.y; y++) {
            TileInfo& tile = m[x][y];

            // solid if on edge or both coordinates are even
            // (isEven(x) && isEven(y)) -- for inner tiles
            bool fixed_solid = (x == 0 || x == mi.coord_dimensions.x - 1 || y == 0 || y == mi.coord_dimensions.y - 1);
            if (fixed_solid)
                tile.value = 1;
            
            // create graphics
            tile.quad_id_lower = core->glenv.genQuad("Quad_Tile",
                Transform{glm::vec3(
                    ((x * mi.unit_pixel_dimensions.x) + (mi.unit_pixel_dimensions.x / 2.0f)) + mi.coord_origin.x,
                    ((y * mi.unit_pixel_dimensions.y) + (mi.unit_pixel_dimensions.y / 2.0f)) + mi.coord_origin.y, 
                    -2.0f
                ), glm::vec3(1.0f)}
            );
            tile.quad_id_upper = core->glenv.genQuad("Quad_Tile",
                Transform{glm::vec3(
                    ((x * mi.unit_pixel_dimensions.x) + (mi.unit_pixel_dimensions.x / 2.0f)) + mi.coord_origin.x,
                    ((y * mi.unit_pixel_dimensions.y) + (mi.unit_pixel_dimensions.y / 2.0f)) + mi.coord_origin.y, 
                    -1.0f
                ), glm::vec3(1.0f)}
            );

            Quad *q_lower = core->glenv.getQuad(tile.quad_id_lower);
            Quad *q_upper = core->glenv.getQuad(tile.quad_id_upper);

            // set lower tile graphic
            if (isEven(x + y))
                q_lower->animationstate().setCycleState("dark_floor");
            else
                q_lower->animationstate().setCycleState("light_floor");

            // set upper tile graphic
            if (tile.value > 0) {
                if (tile.value == 1)
                    q_upper->animationstate().setCycleState("stud");
                else
                    q_upper->animationstate().setCycleState("brick");
            } else
                q_upper->animationstate().setCycleState("air");
            
            q_lower->writeAnimation();
            q_upper->writeAnimation();

            // spawn bricks
            if (!fixed_solid && (rand() % 4 == 0) && false)
                core->entitymanager.spawnEntity("Entity_BreakableTile", Transform{toVec3(mi.toPixels(glm::uvec2(x, y)), 0.0f), glm::vec3(1.0f)});
        }
    }

    // try to place player randomly
    glm::uvec2 player_pos;
    while (true) {
        player_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        player_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[player_pos.x][player_pos.y].value > 0)
            continue;
        core->entitymanager.spawnEntity("Entity_Player", Transform{toVec3(mi.toPixels(player_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }

    // try to place key randomly
    glm::uvec2 key_pos;
    while (true) {
        key_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        key_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[key_pos.x][key_pos.y].value > 0 || key_pos == player_pos)
            continue;
        core->entitymanager.spawnEntity("Entity_Key", Transform{toVec3(mi.toPixels(key_pos), 0.0f), glm::vec3(1.0f)});
        core->globalresources.container_Pickup.getLastInstance()->item_name = "key";
        break;
    }

    // try to place stairs randomly
    glm::uvec2 stairs_pos;
    while (true) {
        stairs_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        stairs_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[stairs_pos.x][stairs_pos.y].value > 0 || stairs_pos == player_pos || stairs_pos == key_pos)
            continue;
        core->entitymanager.spawnEntity("Entity_Stairs", Transform{toVec3(mi.toPixels(stairs_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }

    // try to place enemy randomly
    glm::uvec2 enemy_pos;
    while (true) {
        enemy_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        enemy_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[enemy_pos.x][enemy_pos.y].value > 0 || enemy_pos == player_pos || enemy_pos == key_pos || enemy_pos == stairs_pos)
            continue;
        core->entitymanager.spawnEntity("Entity_BasicEnemy", Transform{toVec3(mi.toPixels(enemy_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }
}