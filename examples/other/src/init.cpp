#include "init.hpp"

using namespace glm;

const unsigned MAX_QUADS = 2048;

const unsigned WINDOW_WIDTH = 512;
const unsigned WINDOW_HEIGHT = 512;
const unsigned VIEW_PIXEL_WIDTH = WINDOW_WIDTH / 2;
const unsigned VIEW_PIXEL_HEIGHT = WINDOW_HEIGHT / 2;
const unsigned PIXEL_LEVELS = 16;

const unsigned TEX_SPACE_WIDTH = 224;
const unsigned TEX_SPACE_HEIGHT = 80;
const unsigned TEX_SPACE_LEVELS = 2;

const unsigned UNIT_PIXEL_WIDTH = 16;
const unsigned UNIT_PIXEL_HEIGHT = 16;

void loadAssets() {
    globalstate.animations["Animation_LightBall"] =
        Animation("Animation_LightBall", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 16.0f, 0.0f), vec2(16.0f), 4)
            })
        });
    globalstate.animations["Animation_SmallLightBall"] =
        Animation("Animation_SmallLightBall", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 16.0f, 0.0f), vec2(16.0f), 4)
            })
        });
    globalstate.animations["Animation_LightParticle"] =
        Animation("Animation_LightParticle", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(32.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(48.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(64.0f, 16.0f, 0.0f), vec2(16.0f), 6)
            })
        });
    globalstate.animations["Animation_Player"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Chaser"] =
        Animation("Animation_Chaser", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Shooter"] =
        Animation("Animation_Shooter", {
            Cycle("default", false, {
                Frame(vec3(32.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_FireBall"] =
        Animation("Animation_FireBall", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 32.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_FireParticle"] =
        Animation("Animation_FireParticle", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 32.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(32.0f, 32.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(48.0f, 32.0f, 0.0f), vec2(16.0f), 6)
            })
        });

    globalstate.filters["Filter_Player"] = 
        Filter("Filter_Player", 0,
            {1, 3},
            {0, 2},
            {}, 
            {}
        );
    globalstate.filters["Filter_Enemy"] = 
    Filter("Filter_Enemy", 1,
        {0, 2},
        {1, 3}, 
        {}, 
        {}
    );
    globalstate.filters["Filter_PlayerProjectile"] = 
    Filter("Filter_PlayerProjectile", 2,
        {1},
        {0, 2, 3}, 
        {}, 
        {}
    );
    globalstate.filters["Filter_EnemyProjectile"] = 
    Filter("Filter_EnemyProjectile", 3,
        {0},
        {1, 2, 3}, 
        {}, 
        {}
    );
    
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    globalstate.glenv.addQuads({
        {glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Player"], "Quad_Player"},
        {glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Chaser"], "Quad_Chaser"},
        {glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Shooter"], "Quad_Shooter"},
        {glm::vec3(0.0f, 0.0f, 1.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightBall"], "Quad_LightBall"},
        {glm::vec3(0.0f, 0.0f, 1.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_SmallLightBall"], "Quad_SmallLightBall"},
        {glm::vec3(0.0f, 0.0f, 0.5f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightParticle"], "Quad_LightParticle"},
        {glm::vec3(0.0f, 0.0f, 0.5f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_FireBall"], "Quad_FireBall"},
        {glm::vec3(0.0f, 0.0f, 0.5f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_FireParticle"], "Quad_FireParticle"}
    });
    
    globalstate.executor.addEntities({
        {&globalstate.resource_RepeatSpawn.allocator, 0, true, {}, {}, {}, "Entity_RepeatSpawn"},
        {&globalstate.resource_Pickup.allocator, 0, true, {}, {}, {}, "Entity_Pickup"},
        {&globalstate.resource_Lifetime.allocator, 0, true, {}, {}, {}, "Entity_Lifetime"},
        {&globalstate.resource_Travel.allocator, 0, true, {}, {}, {}, "Entity_Travel"},
        {&globalstate.resource_AcquirePlayer.allocator, 0, true, {}, {}, {}, "Entity_AcquirePlayer"},
        {&globalstate.resource_Hitbox.allocator, 0, true, {}, {}, {}, "Entity_Hitbox"},
        {&globalstate.resource_Being.allocator, 0, true, {}, {}, {}, "Entity_Being"},
        {&globalstate.resource_Player.allocator, 0, true, {"Entity_Being", "Entity_Hitbox"}, {"Quad_Player"}, {{Transform{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_Player"]}}, "Entity_Player"},
        {&globalstate.resource_LightBall.allocator, 0, true, {"Entity_Lifetime", "Entity_RepeatSpawn", "Entity_Travel", "Entity_Hitbox"}, {"Quad_LightBall"}, {{Transform{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_PlayerProjectile"]}}, "Entity_LightBall"},
        {&globalstate.resource_LightBall.allocator, 0, true, {"Entity_Lifetime", "Entity_RepeatSpawn", "Entity_Travel", "Entity_Hitbox"}, {"Quad_SmallLightBall"}, {{Transform{glm::vec3(0.0f), (0.375f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_PlayerProjectile"]}}, "Entity_SmallLightBall"},
        {&globalstate.resource_LightParticle.allocator, 0, true, {"Entity_Lifetime"}, {"Quad_LightParticle"}, {}, "Entity_LightParticle"},
        {&globalstate.resource_Chaser.allocator, 0, true, {"Entity_Being", "Entity_AcquirePlayer", "Entity_Travel", "Entity_Hitbox"}, {"Quad_Chaser"}, {{Transform{glm::vec3(0.0f), (0.75f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_Enemy"]}}, "Entity_Chaser"},
        {&globalstate.resource_Shooter.allocator, 0, true, {"Entity_Being", "Entity_AcquirePlayer", "Entity_Travel", "Entity_Hitbox"}, {"Quad_Shooter"}, {{Transform{glm::vec3(0.0f), (0.75f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_Enemy"]}}, "Entity_Shooter"},
        {&globalstate.resource_FireBall.allocator, 0, true, {"Entity_Lifetime", "Entity_RepeatSpawn", "Entity_Travel", "Entity_Hitbox"}, {"Quad_FireBall"}, {{Transform{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f)}, &globalstate.filters["Filter_EnemyProjectile"]}}, "Entity_FireBall"},
        {&globalstate.resource_FireParticle.allocator, 0, true, {"Entity_Lifetime"}, {"Quad_FireParticle"}, {}, "Entity_FireParticle"}
    });
}

void initialize() {
    srand(time(NULL));

    // initialize GLFW, OpenGL, and GLFWInput
    globalstate.glfwstate.init(WINDOW_WIDTH, WINDOW_HEIGHT, "title", true);
    globalstate.input.setWindow(globalstate.glfwstate.getWindowHandle(), VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT);
    globalstate.glenv.init(MAX_QUADS);
    globalstate.glenv.setTexArray(TEX_SPACE_WIDTH, TEX_SPACE_HEIGHT, TEX_SPACE_LEVELS);
    globalstate.glenv.setTexture(Image("gfx/sprites.png"), 0, 0, 0);
    globalstate.glenv.setViewTopDown(0.0f, 0.0f, 1.0f);
    globalstate.glenv.setProjOrthographic(VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT, float(PIXEL_LEVELS));

    globalstate.inventory["key"] = 0;
}