#include "loop.hpp"

void loop() {
    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(globalstate.glfwstate.getWindowHandle()) && !globalstate.input.get_esc()) {
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
        if (globalstate.level_clear_started) {
            if (!globalstate.manager.groupSize("Group_Spawnable")) {
                globalstate.level_generated = false;
                globalstate.level_clear_started = false;
            }

        } else {
            // generate level if none generated; else, check if level needs to be cleared
            if (!globalstate.level_generated) {
                genLevel();
                globalstate.level_generated = true;

            } else {
                if (globalstate.stairs_entered) {
                    clearLevel();
                    globalstate.stairs_entered = false;
                    globalstate.level_clear_started = true;
                }
            }
        }

        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        globalstate.collisionspace.detectCollisionAABB();

        globalstate.input.update();

        globalstate.executor.runExecQueue(0);
        globalstate.executor.runExecQueue(1);
        globalstate.executor.runSpawnQueue();
        globalstate.executor.runKillQueue();
        globalstate.executor.runUpdate();

        globalstate.manager.update();

        globalstate.glenv.update();
        globalstate.glenv.drawQuads();

        glfwSwapBuffers(globalstate.glfwstate.getWindowHandle());
    };

    std::cout << "Ending loop" << std::endl;
}

void clearLevel() {
    // remove existing map and entities
    if (!globalstate.level_generated)
        throw std::runtime_error("Attempt to clear level when none is generated");

    // remove existing map and entities
    if (globalstate.level_clear_started)
        throw std::runtime_error("Attempt to clear level when clear is already in progress");
    
    auto& m = globalstate.map;
    auto& mi = globalstate.mapinfo;

    // remove quads and unset tile fields
    for (unsigned x = 0; x < mi.coord_dimensions.x; x++) {
        for (unsigned y = 0; y < mi.coord_dimensions.y; y++) {
            TileInfo& tile = m[x][y];

            if (tile.quad_id_lower >= 0)
                globalstate.glenv.remove(tile.quad_id_lower);
            if (tile.quad_id_upper >= 0)
                globalstate.glenv.remove(tile.quad_id_upper);

            tile.value = -1;
            tile.quad_id_lower = -1;
            tile.quad_id_upper = -1;
        }
    }

    // kill all entities
    for (auto iter = globalstate.manager.groupBegin("Group_Spawnable"); iter != globalstate.manager.groupEnd("Group_Spawnable"); iter++)
        (*iter)->kill();
}

void genLevel() {
    if (globalstate.level_generated)
        throw std::runtime_error("Attempt to generate level when it already exists");

    if (globalstate.level_clear_started)
        throw std::runtime_error("Attempt to generate level when level clearing is in progress");
    
    auto& m = globalstate.map;
    auto& mi = globalstate.mapinfo;

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
            tile.quad_id_lower = globalstate.glenv.genQuad("Quad_Tile",
                Transform{glm::vec3(
                    ((x * mi.unit_pixel_dimensions.x) + (mi.unit_pixel_dimensions.x / 2.0f)) + mi.coord_origin.x,
                    ((y * mi.unit_pixel_dimensions.y) + (mi.unit_pixel_dimensions.y / 2.0f)) + mi.coord_origin.y, 
                    -2.0f
                ), glm::vec3(1.0f)}
            );
            tile.quad_id_upper = globalstate.glenv.genQuad("Quad_Tile",
                Transform{glm::vec3(
                    ((x * mi.unit_pixel_dimensions.x) + (mi.unit_pixel_dimensions.x / 2.0f)) + mi.coord_origin.x,
                    ((y * mi.unit_pixel_dimensions.y) + (mi.unit_pixel_dimensions.y / 2.0f)) + mi.coord_origin.y, 
                    -1.0f
                ), glm::vec3(1.0f)}
            );

            Quad *q_lower = globalstate.glenv.getQuad(tile.quad_id_lower);
            Quad *q_upper = globalstate.glenv.getQuad(tile.quad_id_upper);

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
                globalstate.manager.spawnEntity("Entity_BreakableTile", Transform{toVec3(mi.toPixels(glm::uvec2(x, y)), 0.0f), glm::vec3(1.0f)});
        }
    }

    // try to place player randomly
    glm::uvec2 player_pos;
    while (true) {
        player_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        player_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[player_pos.x][player_pos.y].value > 0)
            continue;
        globalstate.manager.spawnEntity("Entity_Player", Transform{toVec3(mi.toPixels(player_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }

    // try to place key randomly
    glm::uvec2 key_pos;
    while (true) {
        key_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        key_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[key_pos.x][key_pos.y].value > 0 || key_pos == player_pos)
            continue;
        globalstate.manager.spawnEntity("Entity_Key", Transform{toVec3(mi.toPixels(key_pos), 0.0f), glm::vec3(1.0f)});
        globalstate.container_Pickup.getLastInstance()->item_name = "key";
        break;
    }

    // try to place stairs randomly
    glm::uvec2 stairs_pos;
    while (true) {
        stairs_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        stairs_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[stairs_pos.x][stairs_pos.y].value > 0 || stairs_pos == player_pos || stairs_pos == key_pos)
            continue;
        globalstate.manager.spawnEntity("Entity_Stairs", Transform{toVec3(mi.toPixels(stairs_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }

    // try to place enemy randomly
    glm::uvec2 enemy_pos;
    while (true) {
        enemy_pos.x = (rand() % unsigned(mi.coord_dimensions.x - 1)) + 1;
        enemy_pos.y = (rand() % unsigned(mi.coord_dimensions.y - 1)) + 1;
        if (m[enemy_pos.x][enemy_pos.y].value > 0 || enemy_pos == player_pos || enemy_pos == key_pos || enemy_pos == stairs_pos)
            continue;
        globalstate.manager.spawnEntity("Entity_BasicEnemy", Transform{toVec3(mi.toPixels(enemy_pos), 0.0f), glm::vec3(1.0f)});
        break;
    }
}