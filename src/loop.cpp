#include "loop.hpp"

enum Type { T_BASIC_ORBSHOT, T_CHASER_SMALLBALL, T_CHASER_MEDIUMBALL, T_EFFECT_SMALLSMOKE, T_EFFECT_MEDIUMSMOKE, T_EFFECT_ORBSHOTPARTICLE, T_EFFECT_ORBSHOTBOOM, T_EFFECT_BALLPARTICLE, T_EFFECT_RING };

class OrbShot : public Basic {
    int _i;
    int _lifetime;

    void _initBasic() {
        getBox()->mass = 0.0f;
        getQuad()->scale.v = glm::vec3(6.0f, 6.0f, 1.0f);

        _i = 0;
        _lifetime = 119;
        getAnimState().setAnimState(0);
    }

    void _baseBasic() {
        _i++;

        if (_i % 10 == 0) {
            int id = getManager()->spawnEntity("OrbShotParticle");
            if (id >= 0) {
                Entity *particle = getManager()->getEntity(id);
                particle->getQuad()->pos.v = getBox()->pos;
            }
        }

        if (_i >= _lifetime)
            kill();
    }

    void _killBasic() {
        int id = getManager()->spawnEntity("OrbShotBoom");
        if (id >= 0) {
            Entity *boom = getManager()->getEntity(id);
            boom->getQuad()->pos.v = getBox()->pos;
        }
    }

    void _collisionBasic(Box *box) {
        kill();
    }

public:
    OrbShot() : Basic(glm::vec3(6.0f, 6.0f, 0.0f)) {}
};

class SmallSmoke : public Effect {
    void _initEffect() {
        getQuad()->pos.v.z = 1.0f;
        getAnimState().setAnimState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    SmallSmoke() : Effect(glm::vec3(16.0f, 16.0f, 1.0f), 20) {}
};

class MediumSmoke : public Effect {
    void _initEffect() {
        getQuad()->pos.v.z = 1.0f;
        getAnimState().setAnimState(rand() % 2);
    }
    void _baseEffect() {}
    void _killEffect() {}
public:
    MediumSmoke() : Effect(glm::vec3(24.0f, 24.0f, 1.0f), 20) {}
};

class OrbShotParticle : public Effect {
    void _initEffect() { getQuad()->pos.v.z = 1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotParticle() : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 24) {}
};

class OrbShotBoom : public Effect {
    void _initEffect() { getQuad()->pos.v.z = 1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    OrbShotBoom() : Effect(glm::vec3(8.0f, 8.0f, 1.0f), 24) {}
};

class BallParticle : public Effect {
    glm::vec3 _dir;

    void _initEffect() { getQuad()->pos.v.z = 1.0f; }
    void _baseEffect() {
        getQuad()->pos.v = getQuad()->pos.v + _dir;
    }
    void _killEffect() {}
public:
    BallParticle(glm::vec3 dir) : Effect(glm::vec3(6.0f, 6.0f, 1.0f), 18), _dir(dir) {}
};

class Ring : public Effect {
    void _initEffect() { getQuad()->pos.v.z = -1.0f; }
    void _baseEffect() {}
    void _killEffect() {}
public:
    Ring() : Effect(glm::vec3(64.0f, 64.0f, 1.0f), -1) {}
};

Object *OrbShot_allocator() { return new OrbShot; }
Entity *SmallSmoke_allocator() { return new SmallSmoke; }
Entity *MediumSmoke_allocator() { return new MediumSmoke; }
Entity *OrbShotParticle_allocator() { return new OrbShotParticle; }
Entity *OrbShotBoom_allocator() { return new OrbShotBoom; }
Entity *BallParticle_allocator() { return new BallParticle(random_angle(glm::vec3(1.0f, 0.0f, 0.0f), 180)); }
Entity *Ring_allocator() { return new Ring; }

Object *target;
Object *SmallBall_allocator() {
    Chaser *chaser = new Chaser(1, "SmallSmoke");
    chaser->chaserSetTarget(target);
    return chaser;
}
Object *MediumBall_allocator() {
    Chaser *chaser = new Chaser(2, "MediumSmoke");
    chaser->chaserSetTarget(target);
    return chaser;
}

void SmallBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(10.0f, 10.0f, 1.0f);
}

void MediumBall_callback(Object *obj) {
    obj->getBox()->dim = glm::vec3(14.0f, 14.0f, 1.0f);
}

void loop(GLFWwindow *winhandle) {
    srand(time(NULL));
    
    // initialize GLEnv instance
    std::cout << "Setting up glenv" << std::endl;
    GLEnv obj_glenv{2048};

    std::cout << "Setting up texture array" << std::endl;
    obj_glenv.setTexArray(133, 164, 4);
    std::cout << "Setting texture" << std::endl;
    obj_glenv.setTexture(Image("objects.png"), 0, 0, 0);
    obj_glenv.setTexture(Image("effects.png"), 0, 0, 1);
    obj_glenv.setTexture(Image("characters1.png"), 0, 0, 2); // each character is 7x24 pixels
    obj_glenv.setTexture(Image("characters2.png"), 0, 0, 3); // each character is 5x10 pixels
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

    // add objects to manager
    std::cout << "Adding entities and objects to manager" << std::endl;
    obj_manager.addObject(OrbShot_allocator, "OrbShot", T_BASIC_ORBSHOT, true, true, true, true, "OrbShot", true, "ProjectileFriendly", nullptr);

    obj_manager.addObject(SmallBall_allocator, "SmallBall", T_CHASER_SMALLBALL, true, true, true, true, "SmallBall", true, "Enemy", SmallBall_callback);
    obj_manager.addObject(MediumBall_allocator, "MediumBall", T_CHASER_MEDIUMBALL, true, true, true, true, "MediumBall", true, "Enemy", MediumBall_callback);

    obj_manager.addEntity(SmallSmoke_allocator, "SmallSmoke", T_EFFECT_SMALLSMOKE, true, true, true, true, "SmallSmoke", nullptr);
    obj_manager.addEntity(MediumSmoke_allocator, "MediumSmoke", T_EFFECT_MEDIUMSMOKE, true, true, true, true, "MediumSmoke", nullptr);
    obj_manager.addEntity(OrbShotParticle_allocator, "OrbShotParticle", T_EFFECT_ORBSHOTPARTICLE, true, true, true, true, "OrbShotParticle", nullptr);
    obj_manager.addEntity(OrbShotBoom_allocator, "OrbShotBoom", T_EFFECT_ORBSHOTBOOM, true, true, true, true, "OrbShotBoom", nullptr);
    obj_manager.addEntity(BallParticle_allocator, "BallParticle", T_EFFECT_BALLPARTICLE, true, true, true, true, "BallParticle", nullptr);
    obj_manager.addEntity(Ring_allocator, "Ring", T_EFFECT_RING, true, true, true, true, "Ring", nullptr);

    // set up input
    std::cout << "Setting up input" << std::endl;
    Input input(winhandle, pixelwidth, pixelheight);

    // set up ring
    //std::cout << "Setting up ring" << std::endl;
    //obj_manager.spawnEntity("Ring");
    
    // set up player
    std::cout << "Setting up player" << std::endl;
    Player player;
    player.scriptSetup(&obj_executor);
    player.entitySetup(&obj_glenv, &animations["Player"]);
    player.objectSetup(&obj_physenv, &filters["Player"]);
    player.playerSetup(&input, &obj_manager);
    player.enqueue();
    target = &player;

    // text
    std::cout << "Setting up text" << std::endl;
    TextConfig largefont{0, 0, 2, 5, 19, 7, 24, 0, 0, 1};
    TextConfig smallfont{0, 0, 3, 5, 19, 5, 10, 0, 0, 1};
    
    Text text1(&obj_glenv);
    text1.setTextConfig(largefont);
    text1.setText("\"Test!\" Value: 23% (#8)");
    text1.setPos(glm::vec3(0.0f, 32.0f, 1.0f));

    Text text2(&obj_glenv);
    text2.setTextConfig(smallfont);
    text2.setPos(glm::vec3(0.0f, 16.0f, 1.0f));
    /*
    Text is expected to be in the order of ASCII format, starting with the space character (decimal 32).

    - tex_x - X coordinate in the texture space the character sheet is located
    - tex_y - Y coordinate in the texture space the character sheet is located
    - tex_z - Z coordinate in the texture space the character sheet is located
    - tex_rows - number of rows in sheet
    - tex_columns - number of columns in sheet
    - text_width - width of a single character
    - text_height - height of a single character
    - text_xoff - horizontal spacing between characters in sheet
    - text_yoff - vertical spacing between characters in sheet
    - spacing - space to place between characters in final text
    */

    // start loop
    std::cout << "Starting loop" << std::endl;
    int i = 0;
    while (!glfwWindowShouldClose(winhandle)) {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (i % 60 == 0) {
            // get random spawn location along radius
            float spawn_radius = sqrtf((halfwidth * halfwidth) + (halfheight + halfheight)) + 64;
            glm::vec3 spawn_vec(spawn_radius, 0.0f, 0.0f);
            spawn_vec = random_angle(spawn_vec, 180);

            // spawn enemy
            // TODO: fix error when wrong name is used
            int enemy_id;
            if (rand() % 2 == 0)
                enemy_id = obj_manager.spawnObject("SmallBall");
            else
                enemy_id = obj_manager.spawnObject("MediumBall");

            Object *enemy = obj_manager.getObject(enemy_id);
            enemy->getBox()->pos = spawn_vec;
        }
        
        input.update();

        // run global scripts
        //gbl_executor.runExec();
        //gbl_executor.runKill();

        // run object scripts
        obj_executor.runExec();
        obj_executor.runKill();

        // update physics environment and detect collisions
        obj_physenv.step();
        obj_physenv.detectCollision();

        // update text
        text1.update();
        text2.update();

        // update graphic environment and draw
        obj_glenv.update();
        obj_glenv.draw();

        glfwSwapBuffers(winhandle);

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        i++;
    };
    
    // terminate GLFW
    std::cout << "Terminating GLFW" << std::endl;
    glfwDestroyWindow(winhandle);
    glfwTerminate();
}