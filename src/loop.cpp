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
        getBox()->mass = 0.0f;
        getQuad()->bv_scale.v = glm::vec3(6.0f, 6.0f, 1.0f);

        _i = 0;
        _lifetime = 119;
        getAnimState().setCycleState(0);
    }

    void _baseBasic() {
        _i++;

        if (_i % 10 == 0) {
            int id = getManager()->spawnEntity("OrbShotParticle");
            if (id >= 0) {
                Entity *particle = getManager()->getEntity(id);
                particle->getQuad()->bv_pos.v = getBox()->pos;
            }
        }

        if (_i >= _lifetime)
            kill();
    }

    void _killBasic() {
        int id = getManager()->spawnEntity("OrbShotBoom");
        if (id >= 0) {
            Entity *boom = getManager()->getEntity(id);
            boom->getQuad()->bv_pos.v = getBox()->pos;
        }
    }

    void _collisionBasic(Box *box) {
        kill();
    }

public:
    OrbShot() : Basic(glm::vec3(4.0f, 4.0f, 0.0f)) {}
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

Object *OrbShot_allocator() { return new OrbShot; }
Entity *SmallSmoke_allocator() { return new SmallSmoke; }
Entity *MediumSmoke_allocator() { return new MediumSmoke; }
Entity *BigSmoke_allocator() { return new BigSmoke; }
Entity *VeryBigSmoke_allocator() { return new VeryBigSmoke; }
Entity *PlayerSmoke_allocator() { return new PlayerSmoke; }
Entity *OrbShotParticle_allocator() { return new OrbShotParticle; }
Entity *OrbShotBoom_allocator() { return new OrbShotBoom; }
Entity *BallParticle_allocator() { return new BallParticle(random_angle(glm::vec3(1.0f, 0.0f, 0.0f), 180)); }

bool killflag = false;
Entity *Ring_allocator() { 
    return new Ring;
}
Object *SmallBall_allocator() {
    return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), 1, "SmallSmoke", &killflag);
}
Object *MediumBall_allocator() {
    return new Chaser(glm::vec3(16.0f, 16.0f, 1.0f), 2, "MediumSmoke", &killflag);
}
Object *BigBall_allocator() {
    return new Chaser(glm::vec3(20.0f, 20.0f, 1.0f), 3, "BigSmoke", &killflag);
}
Object *VeryBigBall_allocator() {
    return new Chaser(glm::vec3(24.0f, 24.0f, 1.0f), 4, "VeryBigSmoke", &killflag);
}

void SmallBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(10.0f, 10.0f, 1.0f);
}

void MediumBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(14.0f, 14.0f, 1.0f);
}

void BigBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(16.0f, 16.0f, 1.0f);
}

void VeryBigBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(20.0f, 20.0f, 1.0f);
}

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
    std::cout << "Setting up executors" << std::endl;
    Executor gbl_executor{2048};
    Executor obj_executor{2048};

    // set up manager
    std::cout << "Setting up manager" << std::endl;
    ObjectManager obj_manager{2048};
    obj_manager.setExecutor(&obj_executor);
    obj_manager.setGLEnv(&obj_glenv);
    obj_manager.setAnimations(&animations);
    obj_manager.setPhysEnv(&obj_physenv);
    obj_manager.setFilters(&filters);

    // set up input
    std::cout << "Setting up input" << std::endl;
    Input input(winhandle, pixelwidth, pixelheight);

    // set up player_allocator lambda
    auto Player_allocator = [&input]() -> Object* {
        Player *player = new Player;
        player->playerSetup(&input);
        return player;
    };

    // add objects to manager
    std::cout << "Adding entities and objects to manager" << std::endl;
    obj_manager.addObject(OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, true, "OrbShot", "ProjectileFriendly", nullptr);

    obj_manager.addObject(Player_allocator, "Player", T_CHARACTER_PLAYER, true, true, "Player", "Player", nullptr);
    obj_manager.addObject(SmallBall_allocator, "SmallBall", T_CHASER_SMALLBALL, true, true, "SmallBall", "Enemy", SmallBall_callback);
    obj_manager.addObject(MediumBall_allocator, "MediumBall", T_CHASER_MEDIUMBALL, true, true, "MediumBall", "Enemy", MediumBall_callback);
    obj_manager.addObject(BigBall_allocator, "BigBall", T_CHASER_BIGBALL, true, true, "BigBall", "Enemy", BigBall_callback);
    obj_manager.addObject(VeryBigBall_allocator, "VeryBigBall", T_CHASER_VERYBIGBALL, true, true, "VeryBigBall", "Enemy", VeryBigBall_callback);

    obj_manager.addEntity(SmallSmoke_allocator, "SmallSmoke", T_EFFECT_SMALLSMOKE, true, true, "SmallSmoke", nullptr);
    obj_manager.addEntity(MediumSmoke_allocator, "MediumSmoke", T_EFFECT_MEDIUMSMOKE, true, true, "MediumSmoke", nullptr);
    obj_manager.addEntity(BigSmoke_allocator, "BigSmoke", T_EFFECT_BIGSMOKE, true, true, "BigSmoke", nullptr);
    obj_manager.addEntity(VeryBigSmoke_allocator, "VeryBigSmoke", T_EFFECT_VERYBIGSMOKE, true, true, "VeryBigSmoke", nullptr);
    obj_manager.addEntity(PlayerSmoke_allocator, "PlayerSmoke", T_EFFECT_PLAYERSMOKE, true, true, "PlayerSmoke", nullptr);
    obj_manager.addEntity(OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, true, "OrbShotParticle", nullptr);
    obj_manager.addEntity(OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, true, "OrbShotBoom", nullptr);
    obj_manager.addEntity(BallParticle_allocator, "BallParticle", T_EFFECT_BALLPARTICLE, true, true, "BallParticle", nullptr);
    obj_manager.addEntity(Ring_allocator, "Ring", T_EFFECT_RING, true, true, "Ring", nullptr);

    // set up ring
    std::cout << "Setting up ring" << std::endl;
    obj_manager.spawnEntity("Ring");

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

    Text counttext(&obj_glenv);
    counttext.setTextConfig(smallfont);
    counttext.setPos(glm::vec3(96.0f, 96.0f, 1.0f));

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
    
    int player_id = obj_manager.spawnObject("Player");
    Object *player = obj_manager.getObject(player_id);

    bool entercheck = false;
    bool enterstate = false;
    bool debug = false;
    while (!glfwWindowShouldClose(winhandle)) {
        if (debug)
            std::cout << " --- START OF FRAME --- " << std::endl;

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

        if (debug)
            std::cout << "handling game state (= " << game_state << ")" << std::endl;
        
        if (game_state == 0) {
            killflag = true;

            // assume player is dead if address retrieved by ID does not match player address
            if (player != obj_manager.getObject(player_id)) {
                player_id = obj_manager.spawnObject("Player");
                player = obj_manager.getObject(player_id);
            }

            // check for input to start game
            if (enterstate) {
                round = 1;
                killflag = false;
                number = 0.0f;
                rate = 0.0f;
                spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(round)))), 5, 60);

                game_state = 1;
            }
        
        } else if (game_state == 1) {
            if (player == obj_manager.getObject(player_id)) {
                // check distance from center and change rate
                if (glm::length(player->getBox()->pos) < 32.0f)
                    rate += rate_increase;
                else
                    rate += rate_decrease;
                rate = glm::clamp(rate, 0.0f, max_rate);
            } else
                rate = 0.0f;
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
                
                    // spawn enemy (right now only spawns small balls)
                    // TODO: fix error when wrong name is used
                    int enemy_id;
                    switch (int(float(rand() % round) / 5.0f)) {
                        case 0:
                            enemy_id = obj_manager.spawnObject("SmallBall");
                            break;
                        case 1:
                            enemy_id = obj_manager.spawnObject("MediumBall");
                            break;
                        case 2:
                            enemy_id = obj_manager.spawnObject("BigBall");
                            break;
                        case 3:
                            enemy_id = obj_manager.spawnObject("VeryBigBall");
                            break;
                        default:
                            break;
                    }

                    Object *enemy = obj_manager.getObject(enemy_id);
                    enemy->getBox()->pos = spawn_vec1 + spawn_vec2;
                }
            }

            // check if round completed
            if (number == target_number) {
                killflag = true;
                game_state = 2;
            }

            // check if player dead
            if (player != obj_manager.getObject(player_id)) {
                game_state = 3;
            }
        }

        else if (game_state == 2) {
            // check for input to start next round
            if (enterstate) {
                round += 1;
                killflag = false;
                number = 0.0f;
                rate = 0.0f;
                spawn_rate = glm::clamp(int(80.0f - (25.0f * std::log10(float(round)))), 5, 60);

                game_state = 1;
            }
        }

        else if (game_state == 3) {
            // check for input to return to start
            if (enterstate) {
                game_state = 0;
            }
        }

        if (debug)
            std::cout << "setting text based on game state" << std::endl;

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

        //counttext.setText((std::string("count: ") + std::to_string(obj_manager.getCount())).c_str());

        // run object scripts
        if (debug)
            std::cout << "running base scripts" << std::endl;
        
        obj_executor.runExec();
        
        if (debug)
            std::cout << "running kill scripts" << std::endl;
        
        obj_executor.runKill();

        // update physics environment and detect collisions
        if (debug)
            std::cout << "stepping physics" << std::endl;
        
        obj_physenv.step();
        
        if (debug)
            std::cout << "running collision handlers" << std::endl;
        
        obj_physenv.detectCollision();

        // update text
        if (debug)
            std::cout << "updating text" << std::endl;
        
        toptext.update();
        subtext.update();
        bottomtext.update();
        //counttext.update();

        // update graphic environment and draw
        if (debug)
            std::cout << "updating and drawing graphics" << std::endl;
        
        obj_glenv.update();
        obj_glenv.draw();

        if (debug)
            std::cout << "swapping buffers " << std::endl;
        
        glfwSwapBuffers(winhandle);

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i++;
        
        if (debug)
            std::cout << "--- END OF FRAME ---" << std::endl;
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}