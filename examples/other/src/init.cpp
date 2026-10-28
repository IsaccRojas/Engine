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
            Cycle("default", true, {
                Frame(vec3(0.0f, 16.0f, 0.0f), vec2(16.0f), 4),
                Frame(vec3(16.0f, 16.0f, 0.0f), vec2(16.0f), 4)
            })
        });
    globalstate.animations["Animation_LightParticle"] =
        Animation("Animation_LightParticle", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(32.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(48.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(64.0f, 16.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(80.0f, 16.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Player"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Chaser"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    globalstate.filters["Filter_Player"] = 
        Filter("Filter_Player", 0,
            {1},
            {0},
            {}, 
            {}
        );
     globalstate.filters["Filter_Enemy"] = 
        Filter("Filter_PlayerHitbox", 1,
            {0},
            {1}, 
            {}, 
            {}
        );
    
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    globalstate.glenv.addQuads({
        {glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Player"], "Quad_Player"},
        {glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Chaser"], "Quad_Chaser"},
        {glm::vec3(0.0f, 0.0f, 1.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightBall"], "Quad_LightBall"},
        {glm::vec3(0.0f, 0.0f, 0.5f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightParticle"], "Quad_LightParticle"}
    });

    globalstate.collisionspace.addEntityColliders({
        {glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Player"], "EntityCollider_Player"},
        {glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Player"], "EntityCollider_LightBall"},
        {glm::vec3(0.0f), (0.75f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Enemy"], "EntityCollider_Chaser"}
    });
    
    globalstate.executor.addEntities({
        {&globalstate.resource_RepeatSpawn.allocator, 0, true, {}, {}, {}, "Entity_RepeatSpawn"},
        {&globalstate.resource_Pickup.allocator, 0, true, {}, {}, {}, "Entity_Pickup"},
        {&globalstate.resource_Lifetime.allocator, 0, true, {}, {}, {}, "Entity_Lifetime"},
        {&globalstate.resource_Travel.allocator, 0, true, {}, {}, {}, "Entity_Travel"},
        {&globalstate.resource_Hitbox.allocator, 0, true, {}, {}, {}, "Entity_Hitbox"},
        {&globalstate.resource_Being.allocator, 0, true, {}, {}, {}, "Entity_Being"},
        {&globalstate.resource_Player.allocator, 0, true, {"Entity_Being", "Entity_Hitbox"}, {"Quad_Player"}, {"EntityCollider_Player"}, "Entity_Player"},
        {&globalstate.resource_LightBall.allocator, 0, true, {"Entity_Lifetime", "Entity_RepeatSpawn", "Entity_Travel", "Entity_Hitbox"}, {"Quad_LightBall"}, {"EntityCollider_LightBall"}, "Entity_LightBall"},
        {&globalstate.resource_LightParticle.allocator, 0, true, {"Entity_Lifetime"}, {"Quad_LightParticle"}, {}, "Entity_LightParticle"},
        {&globalstate.resource_Enemy_Chaser.allocator, 0, true, {"Entity_Being", "Entity_Travel", "Entity_Hitbox"}, {"Quad_Chaser"}, {"EntityCollider_Chaser"}, "Entity_Enemy_Chaser"}
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