#include "loop.hpp"

Allocators::Allocators(GLFWInput *input, bool *killflag) : 
    Player_allocator(input, &(this->OrbShot_provider)),
    SmallBall_allocator(&(this->Player_provider), killflag),
    MediumBall_allocator(&(this->Player_provider), killflag),
    BigBall_allocator(&(this->Player_provider), killflag),
    VeryBigBall_allocator(&(this->Player_provider), killflag),
    Ring_allocator(&(this->Player_provider))
{}

// need this to initialize Text members
GlobalState::GlobalState(GLEnv *glenv) : toptext(glenv), subtext(glenv), bottomtext(glenv) {}

void loop(CoreResources *core) {
    srand(time(NULL));

    GlobalState globalstate(&core->glenv);
    Allocators allocators(&core->input, &globalstate.killflag);
    allocators.OrbShot_provider.addAllocator(&allocators.OrbShot_allocator, "OrbShot");
    allocators.Player_provider.addAllocator(&allocators.Player_allocator, "Player");

    std::cout << "Setting up allocators and initial game state" << std::endl;
    addAllocators(core, &globalstate, &allocators);
    gameInitialize(core, &globalstate, &allocators);

    std::cout << "Running loop" << std::endl;
    while (!glfwWindowShouldClose(core->state.getWindowHandle())) {
        gameStep(core, &globalstate, &allocators);
        gameProcess(core, &globalstate, &allocators);
    };

    std::cout << "Ending loop" << std::endl;
}

void addAllocators(CoreResources *core, GlobalState *globalstate, Allocators *allocators) {
    // add objects to executor
    core->executor.addObject(&allocators->OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, "OrbShot", "ProjectileFriendly", nullptr, nullptr);
    core->executor.addObject(&allocators->Player_allocator, "Player", T_BASIC_PLAYER, true, "Player", "Player", nullptr, nullptr);
    core->executor.addObject(&allocators->SmallBall_allocator, "SmallBall", T_BASIC_SMALLBALL, true, "SmallBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&allocators->MediumBall_allocator, "MediumBall", T_BASIC_MEDIUMBALL, true, "MediumBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&allocators->BigBall_allocator, "BigBall", T_BASIC_BIGBALL, true, "BigBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&allocators->VeryBigBall_allocator, "VeryBigBall", T_BASIC_VERYBIGBALL, true, "VeryBigBall", "Enemy", nullptr, nullptr);
    core->executor.addEntity(&allocators->SmallSmoke_allocator, "SmallSmoke", T_EFFECT_SMALLSMOKE, true, "SmallSmoke", nullptr, nullptr);
    core->executor.addEntity(&allocators->MediumSmoke_allocator, "MediumSmoke", T_EFFECT_MEDIUMSMOKE, true, "MediumSmoke", nullptr, nullptr);
    core->executor.addEntity(&allocators->BigSmoke_allocator, "BigSmoke", T_EFFECT_BIGSMOKE, true, "BigSmoke", nullptr, nullptr);
    core->executor.addEntity(&allocators->VeryBigSmoke_allocator, "VeryBigSmoke", T_EFFECT_VERYBIGSMOKE, true, "VeryBigSmoke", nullptr, nullptr);
    core->executor.addEntity(&allocators->PlayerSmoke_allocator, "PlayerSmoke", T_EFFECT_PLAYERSMOKE, true, "PlayerSmoke", nullptr, nullptr);
    core->executor.addEntity(&allocators->OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, "OrbShotParticle", nullptr, nullptr);
    core->executor.addEntity(&allocators->OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, "OrbShotBoom", nullptr, nullptr);
    core->executor.addEntity(&allocators->Ring_allocator, "Ring", T_EFFECT_RING, true, "Ring", nullptr, nullptr);
}

void gameInitialize(CoreResources *core, GlobalState *globalstate, Allocators *allocators) {
    core->executor.enqueueSpawnEntity("Ring", 0, -1, glm::vec3(0.0f));

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
    globalstate->target_number = 200.0f;
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
            core->executor.enqueueSpawnObject("Player", 0, -1, glm::vec3(0.0f));
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
            glm::vec3 playerpos = (*(player_set->begin()))->getBox()->pos;

            // check distance from center and change rate
            if (glm::length(playerpos) < 32.0f)
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

                    switch (int(float(rand() % globalstate->round) / 5.0f)) {
                        case 0:
                            core->executor.enqueueSpawnObject("SmallBall", 0, -1, spawn_vec1 + spawn_vec2);
                            break;
                        case 1:
                            core->executor.enqueueSpawnObject("MediumBall", 0, -1, spawn_vec1 + spawn_vec2);
                            break;
                        case 2:
                            core->executor.enqueueSpawnObject("BigBall", 0, -1, spawn_vec1 + spawn_vec2);
                            break;
                        case 3:
                            core->executor.enqueueSpawnObject("VeryBigBall", 0, -1, spawn_vec1 + spawn_vec2);
                            break;
                        default:
                            break;
                    }
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
            core->executor.enqueueSpawnObject("Player", 0, -1, glm::vec3(0.0f));
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
    core->physenv.step();
    core->physenv.unsetCollidedFlags();
    core->physenv.detectCollision();

    // spawn, run execution queue 0, and kill
    core->executor.runSpawnQueue();
    core->executor.runExecQueue(0);
    core->executor.runKillQueue();

    // spawn, run execution queue 1, and kill
    core->executor.runSpawnQueue();
    core->executor.runExecQueue(1);
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