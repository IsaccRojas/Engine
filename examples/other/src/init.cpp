#include "init.hpp"

const char *ANIMATION_DIR = "./animconfig";
const char *FILTER_DIR = "./filterconfig";

const unsigned EXECUTION_QUEUES = 2;
const unsigned MAX_QUADS = 2048;

const unsigned WINDOW_WIDTH = 512;
const unsigned WINDOW_HEIGHT = 512;
const unsigned VIEW_PIXEL_WIDTH = WINDOW_WIDTH / 2;
const unsigned VIEW_PIXEL_HEIGHT = WINDOW_HEIGHT / 2;
const unsigned PIXEL_LEVELS = 16;
const unsigned UNIT_PIXEL_WIDTH = 16;
const unsigned UNIT_PIXEL_HEIGHT = 16;

// tile 0, 0 would be located above and to the right of this position
const unsigned COORD_WIDTH = 15;
const unsigned COORD_HEIGHT = 13;
const int COORD_ORIGIN_PIXEL_X = 0;
const int COORD_ORIGIN_PIXEL_Y = 0;

const unsigned TEX_SPACE_WIDTH = 224;
const unsigned TEX_SPACE_HEIGHT = 80;
const unsigned TEX_SPACE_LEVELS = 2;

CoreResources::CoreResources() :
    entityscriptexecutor(EXECUTION_QUEUES),
    entitymanager(&entityscriptexecutor, &glenv, &collisionspace),
    globalstate(&(this->entitymanager), &(this->entityscriptexecutor), &(this->glfwinput)),
    allocator_Correction(&globalstate),
    allocator_Player(&globalstate),
    allocator_Mover(&globalstate),
    allocator_Pickup(&globalstate),
    allocator_Stairs(&globalstate),
    allocator_BreakableTile(&globalstate),
    allocator_Lifetime(),
    allocator_Spell_LightBallSpell(&globalstate)
{
    allocator_Player.provider().attach(&globalstate.container_Player);
    allocator_Mover.provider().attach(&globalstate.container_Mover);
    allocator_Lifetime.provider().attach(&globalstate.container_Lifetime);
    allocator_Pickup.provider().attach(&globalstate.container_Pickup);
    allocator_Spell_LightBallSpell.provider().attachType<SpellInterface>(&globalstate.container_Spells);
}

void initializeCore(CoreResources *core) {
    // get animation and filter maps
    core->animations = loadAnimations();
    core->filters = loadFilters();

    // initialize GLFW, OpenGL, and GLFWInput
    core->glfwstate.init(WINDOW_WIDTH, WINDOW_HEIGHT, "title", true);
    core->glfwinput.setWindow(core->glfwstate.getWindowHandle(), VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT);
    core->glenv.init(MAX_QUADS);
    core->glenv.setTexArray(TEX_SPACE_WIDTH, TEX_SPACE_HEIGHT, TEX_SPACE_LEVELS);
    core->glenv.setTexture(Image("gfx/sprites.png"), 0, 0, 0);
    core->glenv.setTexture(Image("gfx/tiles.png"), 0, 0, 1);
    core->glenv.setViewTopDown((UNIT_PIXEL_WIDTH * 7) + (UNIT_PIXEL_WIDTH / 2.0f), (UNIT_PIXEL_HEIGHT * 6) + (UNIT_PIXEL_HEIGHT / 2.0f), 1.0f);
    core->glenv.setProjOrthographic(VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT, float(PIXEL_LEVELS));
}

/* Initializes script, graphics, and collision assets.
*/
void initializeAssets(CoreResources *core) {
    srand(time(NULL));
    glm::vec3 unit_scale = glm::vec3(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT, 0.0f);

    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_Player"]}, "Quad_Player");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_BasicEnemy"]}, "Quad_BasicEnemy");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), 2.0f * unit_scale, glm::vec4(1.0f), core->animations["Animation_Slash"]}, "Quad_Slash");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_LightBall"]}, "Quad_LightBall");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_Tile"]}, "Quad_Tile");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_Key"]}, "Quad_Key");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), unit_scale, glm::vec4(1.0f), core->animations["Animation_Stairs"]}, "Quad_Stairs");

    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), core->filters["Filter_Player"]}, "EntityCollider_Player");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), core->filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), (0.85f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), core->filters["Filter_Interactable"]}, "EntityCollider_Interactable");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), (0.5f * unit_scale) + glm::vec3(0.0f, 0.0f, 1.0f), core->filters["Filter_PlayerHitbox"]}, "EntityCollider_PlayerHitbox");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), unit_scale + glm::vec3(0.0f, 0.0f, 1.0f), core->filters["Filter_BreakableTile"]}, "EntityCollider_BreakableTile");

    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Correction, 1, true, nullptr, nullptr}, "ES_Correction");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Player, 0, true, nullptr, nullptr}, "ES_Player");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Mover, 0, true, nullptr, nullptr}, "ES_Mover");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Lifetime, 0, true, nullptr, nullptr}, "ES_Lifetime");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Pickup, 0, true, nullptr, nullptr}, "ES_Pickup");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_Stairs, 0, true, nullptr, nullptr}, "ES_Stairs");
    core->entityscriptexecutor.addEntityScript(EntityScriptInfo{&core->allocator_BreakableTile, 0, false, nullptr, nullptr}, "ES_BreakableTile");
    core->entityscriptexecutor.addScript(ScriptInfo{&core->allocator_Spell_LightBallSpell, 1, true, nullptr, nullptr}, "Spell_LightBallSpell");
    
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Player", "ES_Correction"}, {"Quad_Player"}, {"EntityCollider_Player"}, {{0}}, nullptr}, "Entity_Player");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Mover"}, {"Quad_BasicEnemy"}, {"EntityCollider_Enemy"}, {{0}}, nullptr}, "Entity_BasicEnemy");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {}, {"EntityCollider_PlayerHitbox"}, {{0}}, nullptr}, "Entity_GenericPlayerHitbox");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {"Quad_Slash"}, {}, {}, nullptr}, "Entity_Slash");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Lifetime"}, {"Quad_LightBall"}, {"EntityCollider_PlayerHitbox"}, {{0}}, nullptr}, "Entity_LightBall");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Pickup"}, {"Quad_Key"}, {"EntityCollider_Interactable"}, {{0}}, nullptr}, "Entity_Key");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_Stairs"}, {"Quad_Stairs"}, {"EntityCollider_Interactable"}, {{0}}, nullptr}, "Entity_Stairs");
    core->entitymanager.addEntity(EntityInfo{"Group_Spawnable", {"ES_BreakableTile"}, {"Quad_Tile"}, {"EntityCollider_BreakableTile"}, {{0}}, nullptr}, "Entity_BreakableTile");

    // initialize map
    for (unsigned x = 0; x < COORD_WIDTH; x++) {
        core->globalstate.map.push_back(std::vector<TileInfo>());
        for (unsigned y = 0; y < COORD_HEIGHT; y++)
            core->globalstate.map.back().push_back(TileInfo{-1, -1});
    }
    
    core->globalstate.mapinfo = MapInfo{
        glm::vec2(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT),
        glm::vec2(COORD_WIDTH, COORD_HEIGHT),
        glm::vec2(COORD_ORIGIN_PIXEL_X, COORD_ORIGIN_PIXEL_Y)
    };

    core->globalstate.inventory["key"] = 0;
}