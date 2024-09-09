#include "loop.hpp"

enum Group { 
    T_BASIC_ORBSHOT,
    T_CHARACTER_PLAYER,
    T_CHASER_SMALLBALL, T_CHASER_MEDIUMBALL, T_CHASER_BIGBALL, T_CHASER_VERYBIGBALL, 
    T_EFFECT_SMALLSMOKE, T_EFFECT_MEDIUMSMOKE, T_EFFECT_BIGSMOKE, T_EFFECT_VERYBIGSMOKE, T_EFFECT_PLAYERSMOKE,
    T_EFFECT_ORBSHOTPARTICLE, T_EFFECT_ORBSHOTBOOM, 
    T_EFFECT_BALLPARTICLE, 
    T_EFFECT_RING
};

class OrbShot : public Basic {
    int _i;
    int _lifetime;

    void _initBasic() {
        _i = 0;
        _lifetime = 119;
        getAnimState().setCycleState(0);
    }

    void _baseBasic() {
        _i++;

        if (_i % 10 == 0) {
            getManager()->spawnEntityEnqueue("OrbShotParticle", 0, getBox()->pos);
        }

        if (_i >= _lifetime || getBox()->getCollided())
            enqueueKill();
    }

    void _killBasic() {
        getManager()->spawnEntityEnqueue("OrbShotBoom", 1, getBox()->pos);
    }

    void _collisionBasic(Box *box) {}

public:
    OrbShot() : Basic(glm::vec3(6.0f, 6.0f, 1.0f), glm::vec3(4.0f, 4.0f, 0.0f)) {}
};

class SmallSmoke : public Effect {
    void _initEffect() {
        getQuad()->bv_pos.v.z = 1.0f;
        getAnimState().setCycleState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    SmallSmoke() : Effect(glm::vec3(16.0f, 16.0f, 1.0f), 20) {}
};

class MediumSmoke : public Effect {
    void _initEffect() {
        getQuad()->bv_pos.v.z = 1.0f;
        getAnimState().setCycleState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    MediumSmoke() : Effect(glm::vec3(24.0f, 24.0f, 1.0f), 20) {}
};

class BigSmoke : public Effect {
    void _initEffect() {
        getQuad()->bv_pos.v.z = 1.0f;
        getAnimState().setCycleState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    BigSmoke() : Effect(glm::vec3(26.0f, 26.0f, 1.0f), 20) {}
};

class VeryBigSmoke : public Effect {
    void _initEffect() {
        getQuad()->bv_pos.v.z = 1.0f;
        getAnimState().setCycleState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    VeryBigSmoke() : Effect(glm::vec3(28.0f, 28.0f, 1.0f), 20) {}
};

class PlayerSmoke : public Effect {
    void _initEffect() {
        getQuad()->bv_pos.v.z = 1.0f;
        getAnimState().setCycleState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    PlayerSmoke() : Effect(glm::vec3(16.0f, 16.0f, 1.0f), 20) {}
};

class OrbShotParticle : public Effect {
    void _initEffect() { getQuad()->bv_pos.v.z = 1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotParticle() : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 24) {}
};

class OrbShotBoom : public Effect {
    void _initEffect() { getQuad()->bv_pos.v.z = 1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotBoom() : Effect(glm::vec3(8.0f, 8.0f, 1.0f), 24) {}
};

class BallParticle : public Effect {
    glm::vec3 _dir;

    void _initEffect() { getQuad()->bv_pos.v.z = 1.0f; }
    void _baseEffect() {
        getQuad()->bv_pos.v = getQuad()->bv_pos.v + _dir;
    }
    void _killEffect() {}
public:
    BallParticle(glm::vec3 dir) : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 18), _dir(dir) {}
};

class Ring : public Effect {
    void _initEffect() { getQuad()->bv_pos.v.z = -1.0f; }
    void _baseEffect() {
        std::vector ids = getManager()->getAllByGroup(T_CHARACTER_PLAYER);
        //std::cout << "ids size " << ids.size() << std::endl;
        Entity *player;
        
        getAnimState().setCycleState(0);
        for (unsigned i = 0; i < ids.size(); i++) {
            player = getManager()->getEntity(ids[i]);
            
            // quad position and box position are the same for players, so this is fine
            if (glm::length(player->getQuad()->bv_pos.v - getQuad()->bv_pos.v) < 32.0f) {
                getAnimState().setCycleState(1);
                break;
            }
        }
    }
    void _killEffect() {}
public:
    Ring() : Effect(glm::vec3(64.0f, 64.0f, 1.0f), -1) {}
};

void loop(GLFWwindow *winhandle) {
    srand(time(NULL));
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv obj_glenv{2048};

    std::cout << "Setting up texture array" << std::endl;
    obj_glenv.setTexArray(133, 304, 4);
    std::cout << "Setting texture" << std::endl;
    obj_glenv.setTexture(Image("gfx/objects.png"), 0, 0, 0);
    obj_glenv.setTexture(Image("gfx/effects.png"), 0, 0, 1);
    obj_glenv.setTexture(Image("gfx/characters1.png"), 0, 0, 2); // each character is 7x24 pixels
    obj_glenv.setTexture(Image("gfx/characters2.png"), 0, 0, 3); // each character is 5x10 pixels
    int pixelwidth = 256;
    int pixelheight = 256;
    int pixellayers = 16;

    float halfwidth = float(pixelwidth) / 2.0f;
    float halfheight = float(pixelheight) / 2.0f;

    std::cout << "Setting up view and projection matrices" << std::endl;
    obj_glenv.setView(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    // negative is "further back"
    obj_glenv.setProj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(pixellayers)));

    // set up some opengl parameters
    std::cout << "Setting up some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(0.35f, 0.35f, 0.35f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    
    // initialize animation map
    std::cout << "Setting up animation map" << std::endl;
    std::unordered_map<std::string, Animation> animations = loadAnimations("./animconfig");
    

    // initialize filter map
    std::cout << "Setting up filter map" << std::endl;
    std::unordered_map<std::string, Filter> filters = loadFilters("./filterconfig");

    // initialize PhysEnv instance
    std::cout << "Setting up physenv" << std::endl;
    PhysEnv obj_physenv{2048};

    // set up Executor instance
    std::cout << "Setting up executor" << std::endl;
    Executor obj_executor{2048, 2};

    // set up manager
    std::cout << "Setting up manager" << std::endl;
    ObjectManager obj_manager{2048, &obj_physenv, &filters, &obj_glenv, &animations, &obj_executor};

    // set up input
    std::cout << "Setting up input" << std::endl;
    Input input(winhandle, pixelwidth, pixelheight);

    std::cout << "Setting up allocators" << std::endl;

    GenericObjectAllocator<OrbShot> OrbShot_allocator;
    GenericEntityAllocator<SmallSmoke> SmallSmoke_allocator;
    GenericEntityAllocator<MediumSmoke> MediumSmoke_allocator;
    GenericEntityAllocator<BigSmoke> BigSmoke_allocator;
    GenericEntityAllocator<VeryBigSmoke> VeryBigSmoke_allocator;
    GenericEntityAllocator<PlayerSmoke> PlayerSmoke_allocator;
    GenericEntityAllocator<OrbShotParticle> OrbShotParticle_allocator;
    GenericEntityAllocator<OrbShotBoom> OrbShotBoom_allocator;
    GenericEntityAllocator<Ring> Ring_allocator;

    bool killflag = false;

    // set up player_allocator lambda
    auto Player_allocator = [&input]() -> Object* {
        Player *player = new Player;
        player->playerSetup(&input);
        return player;
    };

    auto SmallBall_allocator = [&killflag]() -> Object* { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(10.0f, 10.0f, 1.0f), 1, "SmallSmoke", &killflag); };
    auto MediumBall_allocator = [&killflag]() -> Object* { return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), glm::vec3(14.0f, 14.0f, 1.0f), 2, "MediumSmoke", &killflag); };
    auto BigBall_allocator = [&killflag]() -> Object* { return new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), glm::vec3(16.0f, 16.0f, 1.0f), 3, "BigSmoke", &killflag); };
    auto VeryBigBall_allocator = [&killflag]() -> Object* { return new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), glm::vec3(20.0f, 20.0f, 1.0f), 4, "VeryBigSmoke", &killflag); };

    // add objects to manager
    std::cout << "Adding entities and objects to manager" << std::endl;
    obj_manager.addObject(&OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, "OrbShot", "ProjectileFriendly", nullptr, nullptr);

    //obj_manager.addObject(Player_allocator, "Player", T_CHARACTER_PLAYER, true, "Player", "Player", nullptr, nullptr);
    //obj_manager.addObject(SmallBall_allocator, "SmallBall", T_CHASER_SMALLBALL, true, "SmallBall", "Enemy", nullptr, nullptr);
    //obj_manager.addObject(MediumBall_allocator, "MediumBall", T_CHASER_MEDIUMBALL, true, "MediumBall", "Enemy", nullptr, nullptr);
    //obj_manager.addObject(BigBall_allocator, "BigBall", T_CHASER_BIGBALL, true, "BigBall", "Enemy", nullptr, nullptr);
    //obj_manager.addObject(VeryBigBall_allocator, "VeryBigBall", T_CHASER_VERYBIGBALL, true, "VeryBigBall", "Enemy", nullptr, nullptr);

    obj_manager.addEntity(&SmallSmoke_allocator, "SmallSmoke", T_EFFECT_SMALLSMOKE, true, "SmallSmoke", nullptr, nullptr);
    obj_manager.addEntity(&MediumSmoke_allocator, "MediumSmoke", T_EFFECT_MEDIUMSMOKE, true, "MediumSmoke", nullptr, nullptr);
    obj_manager.addEntity(&BigSmoke_allocator, "BigSmoke", T_EFFECT_BIGSMOKE, true, "BigSmoke", nullptr, nullptr);
    obj_manager.addEntity(&VeryBigSmoke_allocator, "VeryBigSmoke", T_EFFECT_VERYBIGSMOKE, true, "VeryBigSmoke", nullptr, nullptr);
    obj_manager.addEntity(&PlayerSmoke_allocator, "PlayerSmoke", T_EFFECT_PLAYERSMOKE, true, "PlayerSmoke", nullptr, nullptr);
    obj_manager.addEntity(&OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, "OrbShotParticle", nullptr, nullptr);
    obj_manager.addEntity(&OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, "OrbShotBoom", nullptr, nullptr);
    obj_manager.addEntity(&Ring_allocator, "Ring", T_EFFECT_RING, true, "Ring", nullptr, nullptr);

    // set up ring
    std::cout << "Setting up ring" << std::endl;
    obj_manager.spawnEntity("Ring", 0, glm::vec3(0.0f));

    // text \"Test!\" Value: 23% (#8)
    std::cout << "Setting up text" << std::endl;
    TextConfig largefont{0, 0, 2, 5, 19, 7, 24, 0, 0, 1};
    TextConfig smallfont{0, 0, 3, 5, 19, 5, 10, 0, 0, 1};
    
    Text toptext(&obj_glenv);
    toptext.setTextConfig(largefont);
    toptext.setPos(glm::vec3(0.0f, 96.0f, 1.0f));

    Text subtext(&obj_glenv);
    subtext.setTextConfig(smallfont);
    subtext.setPos(glm::vec3(0.0f, 80.0f, 1.0f));

    Text bottomtext(&obj_glenv);
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
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        input.update();

        // force enter input state to only be true for one frame until released
        if (!entercheck) {
            if (input.get_enter()) {
                enterstate = true;
                entercheck = true;
            } else
                enterstate = false;
        } else {
            enterstate = false;
            if (!input.get_enter()) {
                entercheck = false;
            }
        }
        
        if (game_state == 0) {
            killflag = true;

            // spawn a player if none exist
            if (obj_manager.getAllByGroup(1).size() == 0) {
                obj_manager.spawnObjectEnqueue("Player", 0, glm::vec3(0.0f));
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
            std::vector<unsigned> players = obj_manager.getAllByGroup(1);
            if (players.size() == 0) {
                game_state = 3;

            } else {
                // use first player's position
                glm::vec3 playerpos = obj_manager.getObject(players.at(0))->getBox()->pos;

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
                    float spawn_radius1 = sqrtf((halfwidth * halfwidth) + (halfheight + halfheight)) + 64;
                    glm::vec3 spawn_vec1(spawn_radius1, 0.0f, 0.0f);
                    spawn_vec1 = random_angle(spawn_vec1, 180);

                    // get second random spawn vector with random magnitude
                    int spawn_amount = int(float(rand() % round) / 4.0f) + 1;
                    for (int j = 0; j < spawn_amount; j++) {
                        float spawn_radius2 = float(rand() % 32);
                        glm::vec3 spawn_vec2(spawn_radius2, 0.0f, 0.0f);
                        spawn_vec2 = random_angle(spawn_vec2, 180);

                        switch (int(float(rand() % round) / 5.0f)) {
                            case 0:
                                obj_manager.spawnObjectEnqueue("SmallBall", 0, spawn_vec1 + spawn_vec2);
                                break;
                            case 1:
                                obj_manager.spawnObjectEnqueue("MediumBall", 0, spawn_vec1 + spawn_vec2);
                                break;
                            case 2:
                                obj_manager.spawnObjectEnqueue("BigBall", 0, spawn_vec1 + spawn_vec2);
                                break;
                            case 3:
                                obj_manager.spawnObjectEnqueue("VeryBigBall", 0, spawn_vec1 + spawn_vec2);
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
            if (obj_manager.getAllByGroup(1).size() == 0) {
                obj_manager.spawnObjectEnqueue("Player", 0, glm::vec3(0.0f));
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
        obj_physenv.step();
        obj_physenv.unsetCollidedFlags();
        obj_physenv.detectCollision();

        // spawn, run execution queue 0, and kill
        obj_manager.runSpawnQueue();
        obj_executor.runExecQueue(0);
        obj_executor.runKillQueue();

        // spawn, run execution queue 1, and kill
        obj_manager.runSpawnQueue();
        obj_executor.runExecQueue(1);
        obj_executor.runKillQueue();

        // text updates
        toptext.update();
        subtext.update();
        bottomtext.update();
        
        // graphics updates and draw
        obj_glenv.update();
        obj_glenv.draw();
        
        glfwSwapBuffers(winhandle);
        i++;
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}