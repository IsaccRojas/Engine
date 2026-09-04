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

    globalstate.filters["Filter_Player"] = 
        Filter("Filter_Player", 0,
            {},
            {0, 1}, 
            {}, 
            {}
        );
    globalstate.filters["Filter_PlayerHitbox"] = 
        Filter("Filter_PlayerHitbox", 1,
            {},
            {0, 1}, 
            {}, 
            {}
        );
    
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f, 0.0f, 0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Player"]}, "Quad_Player");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f, 0.0f, 1.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightBall"]}, "Quad_LightBall");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f, 0.0f, 0.5f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightParticle"]}, "Quad_LightParticle");

    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Player"]}, "EntityCollider_Player");
    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_PlayerHitbox"]}, "EntityCollider_LightBall");

    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Player, 0, true, nullptr, nullptr}, "ES_Player");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Lifetime, 0, true, nullptr, nullptr}, "ES_Lifetime");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Pickup, 0, true, nullptr, nullptr}, "ES_Pickup");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_RepeatSpawn, 0, true, nullptr, nullptr}, "ES_RepeatSpawn");
    
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Player"}, {"Quad_Player"}, {"EntityCollider_Player"}, {{0}}, nullptr}, "Entity_Player");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime", "ES_RepeatSpawn"}, {"Quad_LightBall"}, {"EntityCollider_LightBall"}, {{0}}}, "Entity_LightBall");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {"Quad_LightParticle"}, {}, {}}, "Entity_LightParticle");
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