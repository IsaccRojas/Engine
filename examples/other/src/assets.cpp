#include "assets.hpp"

const unsigned UNIT_PIXEL_WIDTH = 16;
const unsigned UNIT_PIXEL_HEIGHT = 16;

using namespace glm;

std::unordered_map<std::string, Animation> loadAnimations() {
    std::unordered_map<std::string, Animation> animations;

    animations["Animation_BasicEnemy"] =
        Animation("Animation_BasicEnemy", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 16.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Key"] =
        Animation("Animation_Key", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_LightBall"] =
        Animation("Animation_LightBall", {
            Cycle("default", true, {
                Frame(vec3(96.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(112.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(128.0f, 0.0f, 0.0f), vec2(16.0f), 6)
            })
        });

    animations["Animation_Player"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Stairs"] =
        Animation("Animation_Stairs", {
            Cycle("default", false, {
                Frame(vec3(32.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            }),
            Cycle("unlocked", false, {
                Frame(vec3(48.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Tile"] =
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
    
    return animations;
}

std::unordered_map<std::string, Filter> loadFilters() {
    std::unordered_map<std::string, Filter> filters;

    filters["Filter_BreakableTile"] = 
        Filter("Filter_BreakableTile", 3,
            {4}, 
            {}, 
            {}, 
            {}
        );

    filters["Filter_Enemy"] = 
        Filter("Filter_Enemy", 1,
            {0, 4},
            {}, 
            {}, 
            {}
        );

    filters["Filter_Interactable"] = 
        Filter("Filter_Interactable", 2,
            {0},
            {}, 
            {}, 
            {}
        );

    filters["Filter_Player"] = 
        Filter("Filter_Player", 0,
            {1, 2},
            {}, 
            {}, 
            {}
        );

    filters["Filter_PlayerHitbox"] = 
        Filter("Filter_PlayerHitbox", 4,
            {1, 3},
            {}, 
            {}, 
            {}
        );
    
    return filters;
}

void loadAssets() {
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Player"]}, "Quad_Player");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_BasicEnemy"]}, "Quad_BasicEnemy");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), 2.0f * unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Slash"]}, "Quad_Slash");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_LightBall"]}, "Quad_LightBall");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Tile"]}, "Quad_Tile");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Key"]}, "Quad_Key");
    globalstate.glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), globalstate.animations["Animation_Stairs"]}, "Quad_Stairs");

    globalstate.collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Player"]}, "EntityCollider_Player");
    globalstate.collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    globalstate.collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), (0.85f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_Interactable"]}, "EntityCollider_Interactable");
    globalstate.collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_PlayerHitbox"]}, "EntityCollider_PlayerHitbox");
    globalstate.collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), globalstate.filters["Filter_BreakableTile"]}, "EntityCollider_BreakableTile");

    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Correction, 1, true, nullptr, nullptr}, "ES_Correction");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Player, 0, true, nullptr, nullptr}, "ES_Player");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Mover, 0, true, nullptr, nullptr}, "ES_Mover");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Lifetime, 0, true, nullptr, nullptr}, "ES_Lifetime");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Pickup, 0, true, nullptr, nullptr}, "ES_Pickup");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_Stairs, 0, true, nullptr, nullptr}, "ES_Stairs");
    globalstate.executor.addEntityScript(EntityScriptInfo{&globalstate.allocator_BreakableTile, 0, false, nullptr, nullptr}, "ES_BreakableTile");
    globalstate.executor.addScript(ScriptInfo{&globalstate.allocator_Spell_LightBallSpell, 1, true, nullptr, nullptr}, "Spell_LightBallSpell");
    
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Player", "ES_Correction"}, {"Quad_Player"}, {"EntityCollider_Player"}, {{0}}, nullptr}, "Entity_Player");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Mover"}, {"Quad_BasicEnemy"}, {"EntityCollider_Enemy"}, {{0}}, nullptr}, "Entity_BasicEnemy");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {}, {"EntityCollider_PlayerHitbox"}, {{0}}, nullptr}, "Entity_GenericPlayerHitbox");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {"Quad_Slash"}, {}, {}, nullptr}, "Entity_Slash");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {"Quad_LightBall"}, {"EntityCollider_PlayerHitbox"}, {{0}}, nullptr}, "Entity_LightBall");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Pickup"}, {"Quad_Key"}, {"EntityCollider_Interactable"}, {{0}}, nullptr}, "Entity_Key");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Stairs"}, {"Quad_Stairs"}, {"EntityCollider_Interactable"}, {{0}}, nullptr}, "Entity_Stairs");
    globalstate.manager.addEntity(EntityInfo{"Group_Spawnable", {"ES_BreakableTile"}, {"Quad_Tile"}, {"EntityCollider_BreakableTile"}, {{0}}, nullptr}, "Entity_BreakableTile");
}