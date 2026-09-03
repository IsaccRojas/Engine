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
    globalstate.animations["Animation_BasicEnemy"] =
        Animation("Animation_BasicEnemy", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 16.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Key"] =
        Animation("Animation_Key", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_LightBall"] =
        Animation("Animation_LightBall", {
            Cycle("default", true, {
                Frame(vec3(96.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(112.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(128.0f, 0.0f, 0.0f), vec2(16.0f), 6)
            })
        });
    globalstate.animations["Animation_Player"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Stairs"] =
        Animation("Animation_Stairs", {
            Cycle("default", false, {
                Frame(vec3(32.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            }),
            Cycle("unlocked", false, {
                Frame(vec3(48.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });
    globalstate.animations["Animation_Tile"] =
        Animation("Animation_Tile", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            }),
            Cycle("dark_floor", false, {
                Frame(vec3(160.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("light_floor", false, {
                Frame(vec3(176.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("stud", false, {
                Frame(vec3(192.0f, 16.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("brick", false, {
                Frame(vec3(208.0f, 16.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("air", false, {
                Frame(vec3(192.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
        });

    globalstate.filters["Filter_BreakableTile"] = 
        Filter("Filter_BreakableTile", 3,
            {4}, 
            {}, 
            {}, 
            {}
        );
    globalstate.filters["Filter_Enemy"] = 
        Filter("Filter_Enemy", 1,
            {0, 4},
            {}, 
            {}, 
            {}
        );
    globalstate.filters["Filter_Interactable"] = 
        Filter("Filter_Interactable", 2,
            {0},
            {}, 
            {}, 
            {}
        );
    globalstate.filters["Filter_Player"] = 
        Filter("Filter_Player", 0,
            {1, 2},
            {}, 
            {}, 
            {}
        );
    globalstate.filters["Filter_PlayerHitbox"] = 
        Filter("Filter_PlayerHitbox", 4,
            {1, 3},
            {}, 
            {}, 
            {}
        );
    
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Player"]}, "Quad_Player");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_BasicEnemy"]}, "Quad_BasicEnemy");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), 2.0f * unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Slash"]}, "Quad_Slash");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightBall"]}, "Quad_LightBall");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Tile"]}, "Quad_Tile");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Key"]}, "Quad_Key");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Stairs"]}, "Quad_Stairs");

    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Player"]}, "EntityCollider_Player");
    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), (0.85f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Interactable"]}, "EntityCollider_Interactable");
    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_PlayerHitbox"]}, "EntityCollider_PlayerHitbox");
    globalstate.collisionspace.addEntityCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_BreakableTile"]}, "EntityCollider_BreakableTile");

    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Player, 0, true, nullptr, nullptr}, "ES_Player");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Lifetime, 0, true, nullptr, nullptr}, "ES_Lifetime");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Pickup, 0, true, nullptr, nullptr}, "ES_Pickup");
    
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Player"}, {"Quad_Player"}, {"EntityCollider_Player"}, {{0}}, nullptr}, "Entity_Player");
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