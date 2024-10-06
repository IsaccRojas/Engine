#include "loop.hpp"

void loop(CoreResources *core) {
    srand(time(NULL));

    // instantiate global state and providers
    GlobalState globalstate(&core->glenv, &core->input);

    // instantiate allocators, and attach them to providers
    ReferrerAllocator<Bullet> Bullet_allocator(&globalstate);
    ReferrerAllocator<Bomb> Bomb_allocator(&globalstate);
    ProvidedEntityAllocator<Explosion> Explosion_allocator;
    ReferrerAllocator<Player> Player_allocator(&globalstate);
    ReferrerAllocator<Enemy> Enemy_allocator(&globalstate);
    ReferrerAllocator<Ring> Ring_allocator(&globalstate);
    ProvidedEntityAllocator<ShrinkParticle> ShrinkParticle_allocator;
    globalstate.providers.Bullet_provider.addAllocator(&Bullet_allocator, "Bullet");
    globalstate.providers.Bomb_provider.addAllocator(&Bomb_allocator, "Bomb");
    globalstate.providers.Explosion_provider.addAllocator(&Explosion_allocator, "Explosion");
    globalstate.providers.Player_provider.addAllocator(&Player_allocator, "Players");
    globalstate.providers.Enemy_provider.addAllocator(&Enemy_allocator, "Enemy");
    globalstate.providers.Ring_provider.addAllocator(&Ring_allocator, "Ring");
    globalstate.providers.ShrinkParticle_provider.addAllocator(&ShrinkParticle_allocator, "ShrinkParticle");

    // add allocators to executor
    core->executor.addEntity(&Bullet_allocator, "Bullet", G_PHYSBALL_BULLET, true, nullptr, nullptr);
    core->executor.addEntity(&Bomb_allocator, "Bomb", G_PHYSBALL_BOMB, true, nullptr, nullptr);
    core->executor.addEntity(&Explosion_allocator, "Explosion", G_PHYSBALL_EXPLOSION, true, nullptr, nullptr);
    core->executor.addEntity(&Player_allocator, "Player", G_PHYSBALL_PLAYER, true, nullptr, nullptr);
    core->executor.addEntity(&Enemy_allocator, "Enemy", G_PHYSBALL_ENEMY, true, nullptr, nullptr);
    core->executor.addEntity(&Ring_allocator, "Ring", G_GFXBALL_RING, true, nullptr, nullptr);
    core->executor.addEntity(&ShrinkParticle_allocator, "ShrinkParticle", G_GFXBALL_SHRINKPARTICLE, true, nullptr, nullptr);

    // subscribe global state to enemies to initialize them
    globalstate.providers.Enemy_provider.subscribe(&globalstate);

    std::cout << "Setting up initial game state" << std::endl;
    gameInitialize(core, &globalstate);

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        gameStep(core, &globalstate);
        gameProcess(core, &globalstate);
    };

    std::cout << "Ending loop" << std::endl;
}

void gameInitialize(CoreResources *core, GlobalState *globalstate) {
    core->executor.enqueueSpawnEntity("Ring", 0, -1, Transform{});

    TextConfig largefont{0, 0, 1, 5, 19, 7, 24, 0, 0, 1};
    TextConfig smallfont{0, 0, 2, 5, 19, 5, 10, 0, 0, 1};
    
    globalstate->toptext.setTextConfig(largefont);
    globalstate->toptext.setPos(glm::vec3(0.0f, 96.0f, 1.0f));

    globalstate->subtext.setTextConfig(smallfont);
    globalstate->subtext.setPos(glm::vec3(0.0f, 80.0f, 1.0f));

    globalstate->bottomtext.setTextConfig(smallfont);
    globalstate->bottomtext.setPos(glm::vec3(0.0f, -80.0f, 1.0f));

    int size = globalstate->upgrade_counts.size();
    float unit_width = 14.0f;
    float unit_height = 14.0f;
    float spacing = 2.0f;
    float set_height = (float(size) * unit_height) + (float(size - 1) * spacing);

    for (int i = 0; i < size; i++) {
        // generate quad
        float x_coord = -126.0f + (unit_width / 2.0f);
        float y_coord = (0.5f * (set_height - unit_height)) - (float(i) * (unit_height + spacing));
        int id = core->glenv.genQuad(glm::vec3(x_coord, y_coord, 1.0f), glm::vec3(14.0f), glm::vec4(1.0f), 0.0f, glm::vec3(0.0f), glm::vec2(0.0f), GLE_RECT);
        Quad *quad = core->glenv.getQuad(id);

        // set animation and cycle state
        quad->animationstate().setAnimation(&(core->animations["Upgrade"]));
        quad->animationstate().setCycleState(i);
        
        // write animation data and update buffers
        quad->writeAnimation();
        quad->update();

        // push text and set it up
        globalstate->upgrade_texts.push_back(Text(&(core->glenv)));
        globalstate->upgrade_texts.back().setTextConfig(smallfont);
        globalstate->upgrade_texts.back().setPos(glm::vec3(x_coord + unit_width + 6.0f, y_coord, 1.0f));
    }
}

void gameStep(CoreResources *core, GlobalState *globalstate) {
    // force enter input state to only be true for one frame until released
    if (!globalstate->enter_check) {
        if (core->input.get_enter()) {
            globalstate->enter_state = true;
            globalstate->enter_check = true;
        } else
            globalstate->enter_state = false;
    } else {
        globalstate->enter_state = false;
        if (!core->input.get_enter()) {
            globalstate->enter_check = false;
        }
    }
    
    if (globalstate->game_state == 0) {
        // spawn a player if none exist
        if (globalstate->providers.Player_provider.getProvidedCount() == 0) {
            core->executor.enqueueSpawnEntity("Player", 0, 65536, Transform{});
        }

        // check for input to start game if at least one player is spawned
        else if (globalstate->enter_state)
            globalstate->transition(1);
    
    } else if (globalstate->game_state == 1) {
        // check if there are no players
        auto player_set = globalstate->providers.Player_provider.getAllProvided();
        
        if (player_set->size() == 0) {
            globalstate->transition(3);

        } else {
            // use first player's position
            glm::vec3 playerpos = (*(player_set->begin()))->transform.pos;

            // check distance from center and change rate
            if (glm::length(playerpos) < 64.0f)
                globalstate->rate += globalstate->rate_increase;
            else
                globalstate->rate += globalstate->rate_decrease;
            globalstate->rate = glm::clamp(globalstate->rate, 0.0f, globalstate->max_rate);

            globalstate->number += globalstate->rate;
            globalstate->number = glm::clamp(globalstate->number, 0.0f, globalstate->target_number);

            // spawn enemies
            if (globalstate->i % globalstate->spawn_rate == 0) {

                // get first random spawn vector with magnitude based on pixel space
                float spawn_radius1 = sqrtf((128.0f * 128.0f) + (128.0f * 128.0f)) + 32.0f;
                glm::vec3 spawn_vec1(spawn_radius1, 0.0f, 0.0f);
                spawn_vec1 = random_angle(spawn_vec1, 180.0f);

                // get second random spawn vector with random magnitude
                int spawn_amount = int(float(rand() % globalstate->round) / 4.0f) + 1;
                for (int j = 0; j < spawn_amount; j++) {
                    float spawn_radius2 = float(rand() % 32);
                    glm::vec3 spawn_vec2(spawn_radius2, 0.0f, 0.0f);
                    spawn_vec2 = random_angle(spawn_vec2, 180.0f);

                    // get random size factor, and store factor
                    int size_factor = int(float(rand() % globalstate->round) / 4.0f);
                    core->executor.enqueueSpawnEntity("Enemy", 0, 65536, Transform{spawn_vec1 + spawn_vec2, glm::vec3(12.0f + (2.0f * float(size_factor)))});
                    globalstate->size_factors.push(size_factor);
                }
            }

            // check if round completed
            if (globalstate->number == globalstate->target_number)
                globalstate->transition(2);
        }
    }

    else if (globalstate->game_state == 2) {
        // respawn player if somehow got here and there are no players
        if (globalstate->providers.Player_provider.getProvidedCount() == 0) {
            core->executor.enqueueSpawnEntity("Player", 0, 65536, Transform{});
        } 

        // check for input to start next round if at least one player is spawned
        else if (globalstate->enter_state) {
            globalstate->transition(1);
        }
    }

    else if (globalstate->game_state == 3) {
        // check for input to return to start
        if (globalstate->enter_state) {
            globalstate->transition(0);
        }
    }

    // construct strings
    float percent = 100.0f * (globalstate->number / globalstate->target_number);
    std::stringstream stream;
    stream << std::fixed << std::setprecision(1) << percent;

    switch (globalstate->game_state) {
        case 0: 
            globalstate->toptext.setText("Ball Game");
            globalstate->subtext.setText("");
            globalstate->bottomtext.setText("Press ENTER to start the game.");
            break;
        case 1:
            globalstate->toptext.setText((std::string("Round ") + std::to_string(globalstate->round)).c_str());
            globalstate->subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
            globalstate->bottomtext.setText("");
            break;
        case 2:
            globalstate->toptext.setText((std::string("Round ") + std::to_string(globalstate->round) + std::string(" complete!")).c_str());
            globalstate->subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
            globalstate->bottomtext.setText("Press ENTER to start the next round.");
            break;
        case 3:
            globalstate->toptext.setText((std::string("Round ") + std::to_string(globalstate->round) + std::string(" failed.")).c_str());
            globalstate->subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
            globalstate->bottomtext.setText("Press ENTER to return to title.");
            break;
        default:
            break;
    }

    for (int i = 0; i < globalstate->upgrade_texts.size(); i++)
        globalstate->upgrade_texts[i].setText((std::string("x") + std::to_string(globalstate->upgrade_counts[i])).c_str());

    globalstate->i++;
}

void gameProcess(CoreResources *core, GlobalState *globalstate) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update inputs
    core->input.update();
    
    // unset collided flags, and perform collision detection
    core->box_space.resetCollidedCount();
    core->box_space.detectCollision();
    core->sphere_space.resetCollidedCount();
    core->sphere_space.detectCollision();

    // spawn anything enqueued by previous step
    core->executor.runSpawnQueue();

    // run execution queue 0, spawn anything
    core->executor.runExecQueue(0);
    core->executor.runSpawnQueue();

    // run execution queue 1, spawn anything
    core->executor.runExecQueue(1);
    core->executor.runSpawnQueue();

    // kill
    core->executor.runKillQueue();
    
    // text updates
    globalstate->toptext.writeText();
    globalstate->subtext.writeText();
    globalstate->bottomtext.writeText();

    for (int i = 0; i < globalstate->upgrade_texts.size(); i++)
        globalstate->upgrade_texts[i].writeText();
    
    // graphics updates and draw
    core->glenv.update();
    core->glenv.drawQuads();

    glfwSwapBuffers(core->state.getWindowHandle());
}