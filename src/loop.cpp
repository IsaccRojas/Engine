#include "loop.hpp"

class SmallBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(10.0f, 10.0f, 1.0f), 1, "SmallSmoke", _killflag); }
public:
    SmallBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class MediumBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(14.0f, 14.0f, 1.0f), 2, "MediumSmoke", _killflag); }
public:
    MediumBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class BigBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), glm::vec3(16.0f, 16.0f, 1.0f), 3, "BigSmoke", _killflag); }
public:
    BigBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class VeryBigBallAllocator : public ObjectAllocatorInterface {
    bool *_killflag;
    Chaser *_allocate(int tag) override { return new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), glm::vec3(20.0f, 20.0f, 1.0f), 4, "VeryBigSmoke", _killflag); }
public:
    VeryBigBallAllocator(bool *killflag) : _killflag(killflag) {}
};

class OrbShotAllocator : public ObjectProvider<OrbShot> {};

class PlayerAllocator : public ObjectAllocatorInterface {
    GLFWInput *_input;
    OrbShotAllocator *_orbshot_allocator;
    Player *_allocate(int tag) override {
        Player *p = new Player(_input); 
        _orbshot_allocator->subscribe(p);
        return p;
    }
public:
    PlayerAllocator(GLFWInput *input, OrbShotAllocator *orbshot_allocator) : _input(input), _orbshot_allocator(orbshot_allocator) {}
};

void loop(CoreResources *core) {
    srand(time(NULL));

    std::cout << "Setting up allocators" << std::endl;
    bool killflag = false;
    GenericEntityAllocator<SmallSmoke> SmallSmoke_allocator;
    GenericEntityAllocator<MediumSmoke> MediumSmoke_allocator;
    GenericEntityAllocator<BigSmoke> BigSmoke_allocator;
    GenericEntityAllocator<VeryBigSmoke> VeryBigSmoke_allocator;
    GenericEntityAllocator<PlayerSmoke> PlayerSmoke_allocator;
    GenericEntityAllocator<OrbShotParticle> OrbShotParticle_allocator;
    GenericEntityAllocator<OrbShotBoom> OrbShotBoom_allocator;
    GenericEntityAllocator<Ring> Ring_allocator;
    OrbShotAllocator OrbShot_allocator;
    PlayerAllocator Player_allocator(&core->input, &OrbShot_allocator);
    SmallBallAllocator SmallBall_allocator(&killflag);
    MediumBallAllocator MediumBall_allocator(&killflag);
    BigBallAllocator BigBall_allocator(&killflag);
    VeryBigBallAllocator VeryBigBall_allocator(&killflag);

    // add objects to executor
    std::cout << "Adding entities and objects to executor" << std::endl;
    core->executor.addObject(&OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, "OrbShot", "ProjectileFriendly", nullptr, nullptr);
    core->executor.addObject(&Player_allocator, "Player", T_BASIC_PLAYER, true, "Player", "Player", nullptr, nullptr);
    core->executor.addObject(&SmallBall_allocator, "SmallBall", T_BASIC_SMALLBALL, true, "SmallBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&MediumBall_allocator, "MediumBall", T_BASIC_MEDIUMBALL, true, "MediumBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&BigBall_allocator, "BigBall", T_BASIC_BIGBALL, true, "BigBall", "Enemy", nullptr, nullptr);
    core->executor.addObject(&VeryBigBall_allocator, "VeryBigBall", T_BASIC_VERYBIGBALL, true, "VeryBigBall", "Enemy", nullptr, nullptr);
    core->executor.addEntity(&SmallSmoke_allocator, "SmallSmoke", T_EFFECT_SMALLSMOKE, true, "SmallSmoke", nullptr, nullptr);
    core->executor.addEntity(&MediumSmoke_allocator, "MediumSmoke", T_EFFECT_MEDIUMSMOKE, true, "MediumSmoke", nullptr, nullptr);
    core->executor.addEntity(&BigSmoke_allocator, "BigSmoke", T_EFFECT_BIGSMOKE, true, "BigSmoke", nullptr, nullptr);
    core->executor.addEntity(&VeryBigSmoke_allocator, "VeryBigSmoke", T_EFFECT_VERYBIGSMOKE, true, "VeryBigSmoke", nullptr, nullptr);
    core->executor.addEntity(&PlayerSmoke_allocator, "PlayerSmoke", T_EFFECT_PLAYERSMOKE, true, "PlayerSmoke", nullptr, nullptr);
    core->executor.addEntity(&OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, "OrbShotParticle", nullptr, nullptr);
    core->executor.addEntity(&OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, "OrbShotBoom", nullptr, nullptr);
    core->executor.addEntity(&Ring_allocator, "Ring", T_EFFECT_RING, true, "Ring", nullptr, nullptr);

    // set up ring
    std::cout << "Setting up ring" << std::endl;
    core->executor.enqueueSpawnEntity("Ring", 0, -1, glm::vec3(0.0f));

    // text \"Test!\" Value: 23% (#8)
    std::cout << "Setting up text" << std::endl;
    TextConfig largefont{0, 0, 2, 5, 19, 7, 24, 0, 0, 1};
    TextConfig smallfont{0, 0, 3, 5, 19, 5, 10, 0, 0, 1};
    
    Text toptext(&core->glenv);
    toptext.setTextConfig(largefont);
    toptext.setPos(glm::vec3(0.0f, 96.0f, 1.0f));

    Text subtext(&core->glenv);
    subtext.setTextConfig(smallfont);
    subtext.setPos(glm::vec3(0.0f, 80.0f, 1.0f));

    Text bottomtext(&core->glenv);
    bottomtext.setTextConfig(smallfont);
    bottomtext.setPos(glm::vec3(0.0f, -80.0f, 1.0f));

    // start loop
    std::cout << "Starting loop" << std::endl;
    
    /* 
    0: game start
    1: round active
    2: round inactive (complete)
    3: round inactive (failed)
    */
    int game_state = 0;
    int i = 0;
    float number = 0.0f;
    float rate = 0.0f;
    float target_number = 200.0f;
    float max_rate = 0.09375f;
    float rate_increase = 0.00035f;
    float rate_decrease = -0.0007f;
    int spawn_rate = 60;
    int round = 0;
    bool entercheck = false;
    bool enterstate = false;
    while (!glfwWindowShouldClose(core->state.getWindowHandle())) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        core->input.update();

        // force enter input state to only be true for one frame until released
        if (!entercheck) {
            if (core->input.get_enter()) {
                enterstate = true;
                entercheck = true;
            } else
                enterstate = false;
        } else {
            enterstate = false;
            if (!core->input.get_enter()) {
                entercheck = false;
            }
        }
        
        if (game_state == 0) {
            killflag = true;

            // spawn a player if none exist
            if (core->executor.getAllByGroup(1).size() == 0) {
                core->executor.enqueueSpawnObject("Player", 0, -1, glm::vec3(0.0f));
            } 
            
            // check for input to start game if at least one player is spawned
            else if (enterstate) {
                round = 1;
                number = 0.0f;
                rate = 0.0f;
                spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(round)))), 5, 60);

                game_state = 1;
            }
        
        } else if (game_state == 1) {
            killflag = false;

            // check if there are no players
            std::vector<unsigned> players = core->executor.getAllByGroup(1);
            if (players.size() == 0) {
                game_state = 3;

            } else {
                // use first player's position
                glm::vec3 playerpos = core->executor.getObject(players.at(0))->getBox()->pos;

                // check distance from center and change rate
                if (glm::length(playerpos) < 32.0f)
                    rate += rate_increase;
                else
                    rate += rate_decrease;
                rate = glm::clamp(rate, 0.0f, max_rate);

                number += rate;
                number = glm::clamp(number, 0.0f, target_number);

                // spawn enemies
                if (i % spawn_rate == 0) {

                    // get first random spawn vector with magnitude based on pixel space
                    float spawn_radius1 = sqrtf((128.0f * 128.0f) + (128.0f * 128.0f)) + 64.0f;
                    glm::vec3 spawn_vec1(spawn_radius1, 0.0f, 0.0f);
                    spawn_vec1 = random_angle(spawn_vec1, 180.0f);

                    // get second random spawn vector with random magnitude
                    int spawn_amount = int(float(rand() % round) / 4.0f) + 1;
                    for (int j = 0; j < spawn_amount; j++) {
                        float spawn_radius2 = float(rand() % 32);
                        glm::vec3 spawn_vec2(spawn_radius2, 0.0f, 0.0f);
                        spawn_vec2 = random_angle(spawn_vec2, 180.0f);

                        switch (int(float(rand() % round) / 5.0f)) {
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
                if (number == target_number) {
                    game_state = 2;
                }
            }
        }

        else if (game_state == 2) {
            killflag = true;

            // respawn player if somehow got here and there are no players
            if (core->executor.getAllByGroup(1).size() == 0) {
                core->executor.enqueueSpawnObject("Player", 0, -1, glm::vec3(0.0f));
            } 

            // check for input to start next round if at least one player is spawned
            else if (enterstate) {
                round += 1;
                number = 0.0f;
                rate = 0.0f;
                spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(round)))), 5, 60);

                game_state = 1;
            }
        }

        else if (game_state == 3) {
            killflag = false;

            // check for input to return to start
            if (enterstate) {
                game_state = 0;
            }
        }

        // construct strings
        float percent = 100.0f * (number / target_number);
        std::stringstream stream;
        stream << std::fixed << std::setprecision(1) << percent;
        
        switch (game_state) {
            case 0: 
                toptext.setText("Ball Game");
                subtext.setText("");
                bottomtext.setText("Press ENTER to start the game.");
                break;
            case 1:
                toptext.setText((std::string("Round ") + std::to_string(round)).c_str());
                subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
                bottomtext.setText("");
                break;
            case 2:
                toptext.setText((std::string("Round ") + std::to_string(round) + std::string(" complete!")).c_str());
                subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
                bottomtext.setText("Press ENTER to start the next round.");
                break;
            case 3:
                toptext.setText((std::string("Round ") + std::to_string(round) + std::string(" failed.")).c_str());
                subtext.setText((std::string("Goal: ") + stream.str() + std::string("%")).c_str());
                bottomtext.setText("Press ENTER to return to title.");
                break;
            default:
                break;
        }
        
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
        toptext.update();
        subtext.update();
        bottomtext.update();
        
        // graphics updates and draw
        core->glenv.update();
        core->glenv.draw();
        
        glfwSwapBuffers(core->state.getWindowHandle());
        i++;
    };

    std::cout << "Ending loop" << std::endl;
}