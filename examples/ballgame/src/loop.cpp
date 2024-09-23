#include "loop.hpp"

Allocators::Allocators(GLFWInput *input, bool *killflag) :
    Bullet_allocator(&(this->ShrinkParticle_provider)),
    Player_allocator(input, &(this->Bullet_provider), &(this->ShrinkParticle_provider)),
    Enemy_allocator(&(this->Player_provider), &(this->ShrinkParticle_provider), killflag),
    Ring_allocator(&(this->Player_provider))
{}

// need this to initialize Text members
GlobalState::GlobalState(GLEnv *glenv) : toptext(glenv), subtext(glenv), bottomtext(glenv) {
    setChannel(65536);
    enableReception(true);
}

// set health of created enemies
void GlobalState::_receive(Enemy *enemy) {
    if (!size_factors.empty()) {
        int size_factor = size_factors.front();
        enemy->setHealth(1.0f + float(size_factor));
        size_factors.pop();
    }
}

void loop(CoreResources *core) {
    srand(time(NULL));

    GlobalState globalstate(&core->glenv);
    Allocators allocators(&core->input, &globalstate.killflag);
    
    // store allocators into providers to intercept their allocations
    allocators.Bullet_provider.addAllocator(&allocators.Bullet_allocator, "Bullet");
    allocators.Player_provider.addAllocator(&allocators.Player_allocator, "Player");
    allocators.Enemy_provider.addAllocator(&allocators.Enemy_allocator, "Enemy");
    allocators.ShrinkParticle_provider.addAllocator(&allocators.ShrinkParticle_allocator, "ShrinkParticle");
    allocators.Enemy_provider.subscribe(&globalstate);

    std::cout << "Setting up allocators and initial game state" << std::endl;
    addAllocators(core, &globalstate, &allocators);
    gameInitialize(core, &globalstate, &allocators);

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle()) && !core->input.get_esc()) {
        gameStep(core, &globalstate, &allocators);
        gameProcess(core, &globalstate, &allocators);
    };

    std::cout << "Ending loop" << std::endl;
}

void addAllocators(CoreResources *core, GlobalState *globalstate, Allocators *allocators) {
    // add objects to executor
    core->executor.addEntity(&allocators->Bullet_allocator, "Bullet", G_PHYSBALL_BULLET, true, nullptr, nullptr);
    core->executor.addEntity(&allocators->Player_allocator, "Player", G_PHYSBALL_PLAYER, true, nullptr, nullptr);
    core->executor.addEntity(&allocators->Enemy_allocator, "Enemy", G_PHYSBALL_ENEMY, true, nullptr, nullptr);
    core->executor.addEntity(&allocators->Ring_allocator, "Ring", G_GFXBALL_RING, true, nullptr, nullptr);
    core->executor.addEntity(&allocators->ShrinkParticle_allocator, "ShrinkParticle", G_GFXBALL_SHRINKPARTICLE, true, nullptr, nullptr);
}

void gameInitialize(CoreResources *core, GlobalState *globalstate, Allocators *allocators) {
    core->executor.enqueueSpawnEntity("Ring", 0, -1, Transform{});

    TextConfig largefont{0, 0, 2, 5, 19, 7, 24, 0, 0, 1};
    TextConfig smallfont{0, 0, 3, 5, 19, 5, 10, 0, 0, 1};
    
    globalstate->toptext.setTextConfig(largefont);
    globalstate->toptext.setPos(glm::vec3(0.0f, 96.0f, 1.0f));

    globalstate->subtext.setTextConfig(smallfont);
    globalstate->subtext.setPos(glm::vec3(0.0f, 80.0f, 1.0f));

    globalstate->bottomtext.setTextConfig(smallfont);
    globalstate->bottomtext.setPos(glm::vec3(0.0f, -80.0f, 1.0f));
    
    /* 
    0: game start
    1: round active
    2: round inactive (complete)
    3: round inactive (failed)
    */
    globalstate->game_state = 0;
    globalstate->i = 0;
    globalstate->number = 0.0f;
    globalstate->rate = 0.0f;
    globalstate->target_number = 300.0f;
    globalstate->max_rate = 0.09375f;
    globalstate->rate_increase = 0.00035f;
    globalstate->rate_decrease = -0.0007f;
    globalstate->spawn_rate = 60;
    globalstate->round = 0;
    globalstate->enter_check = false;
    globalstate->enter_state = false;
}

void gameStep(CoreResources *core, GlobalState *globalstate, Allocators *allocators) {
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
        globalstate->killflag = true;

        // spawn a player if none exist
        if (allocators->Player_provider.getProvidedCount() == 0) {
            core->executor.enqueueSpawnEntity("Player", 0, -1, Transform{});
        }

        // check for input to start game if at least one player is spawned
        else if (globalstate->enter_state) {
            globalstate->round = 1;
            globalstate->number = 0.0f;
            globalstate->rate = 0.0f;
            globalstate->spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(globalstate->round)))), 5, 60);

            globalstate->game_state = 1;
        }
    
    } else if (globalstate->game_state == 1) {
        globalstate->killflag = false;

        // check if there are no players
        auto player_set = allocators->Player_provider.getAllProvided();
        
        if (player_set->size() == 0) {
            globalstate->game_state = 3;

        } else {
            // use first player's position
            glm::vec3 playerpos = (*(player_set->begin()))->transform.pos;

            // check distance from center and change rate
            if (glm::length(playerpos) < 48.0f)
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
            if (globalstate->number == globalstate->target_number) {
                globalstate->game_state = 2;
            }
        }
    }

    else if (globalstate->game_state == 2) {
        globalstate->killflag = true;

        // respawn player if somehow got here and there are no players
        if (allocators->Player_provider.getProvidedCount() == 0) {
            core->executor.enqueueSpawnEntity("Player", 0, -1, Transform{});
        } 

        // check for input to start next round if at least one player is spawned
        else if (globalstate->enter_state) {
            globalstate->round += 1;
            globalstate->number = 0.0f;
            globalstate->rate = 0.0f;
            globalstate->spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(globalstate->round)))), 5, 60);

            globalstate->game_state = 1;
        }
    }

    else if (globalstate->game_state == 3) {
        globalstate->killflag = false;

        // check for input to return to start
        if (globalstate->enter_state) {
            globalstate->game_state = 0;
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

    globalstate->i++;
}

void gameProcess(CoreResources *core, GlobalState *state, Allocators *allocators) {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update inputs
    core->input.update();
    
    // unset collided flags, and perform collision detection
    core->physenv.unsetCollidedFlags();
    core->physenv.detectCollision();

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
    state->toptext.update();
    state->subtext.update();
    state->bottomtext.update();
    
    // graphics updates and draw
    core->glenv.update();
    core->glenv.drawQuads();

    glfwSwapBuffers(core->state.getWindowHandle());
}