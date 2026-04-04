#include "init.hpp"

const char *ANIMATION_DIR = "./animconfig";
const char *FILTER_DIR = "./filterconfig";

const unsigned MAX_COUNT = 2048;
const unsigned EXECUTION_QUEUES = 2;

const unsigned WINDOW_WIDTH = 512;
const unsigned WINDOW_HEIGHT = 512;
const unsigned PIXEL_WIDTH = WINDOW_WIDTH / 2;
const unsigned PIXEL_HEIGHT = WINDOW_HEIGHT / 2;
const unsigned PIXEL_LEVELS = 16;

const unsigned TEX_SPACE_WIDTH = 96;
const unsigned TEX_SPACE_HEIGHT = 64;
const unsigned TEX_SPACE_LEVELS = 3;

const float CLEAR_COLOR_GRAY = 0.0f;

CoreResources::CoreResources() : provider_ES_Player(&(this->globalresources)), globalresources(&(this->input)) {}

void initializeCore(CoreResources *core) {
    // initialize GLFW, OpenGL, and GLFWInput
    std::cout << "Setting up GLFWState" << std::endl;
    core->state.init(WINDOW_WIDTH, WINDOW_HEIGHT, "title", true);

    std::cout << "Setting up OpenGL" << std::endl;
    GLUtil::glinit(true);

    std::cout << "Setting up GLFWInput" << std::endl;
    core->input.setWindow(core->state.getWindowHandle(), PIXEL_WIDTH, PIXEL_HEIGHT);

    // get animation and filter maps
    std::cout << "Loading Animations and Filters" << std::endl;
    core->animations = loadAnimations(ANIMATION_DIR);
    core->filters = loadFilters(FILTER_DIR);

    // set up Executor
    std::cout << "Setting up EntityExecutor" << std::endl;
    core->executor.init(EXECUTION_QUEUES);

    // set up GLEnv
    std::cout << "Setting up GLEnv" << std::endl;
    core->glenv.init(MAX_COUNT);
    core->glenv.setTexArray(TEX_SPACE_WIDTH, TEX_SPACE_HEIGHT, TEX_SPACE_LEVELS);
    core->glenv.setTexture(Image("gfx/sprites2.png"), 0, 0, 0);
    
    float halfwidth = float(PIXEL_WIDTH) / 2.0f;
    float halfheight = float(PIXEL_HEIGHT) / 2.0f;
    core->glenv.setView(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    core->glenv.setProj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(PIXEL_LEVELS)));
    core->glenv.setWindowSpace(WINDOW_WIDTH, WINDOW_HEIGHT);
    core->glenv.setPixelSpace(PIXEL_WIDTH, PIXEL_HEIGHT, PIXEL_LEVELS);

    // set up CollisionSpace
    std::cout << "Setting up CollisionSpace" << std::endl;
    core->collisionspace.init();

    // set up Manager
    std::cout << "Setting up EntityManager" << std::endl;
    core->entitymanager.init(&core->executor, &core->glenv, &core->collisionspace);

    std::cout << "Setting some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(CLEAR_COLOR_GRAY, CLEAR_COLOR_GRAY, CLEAR_COLOR_GRAY, 0.0f);
    glEnable(GL_DEPTH_TEST);
}

/* Initializes script, graphics, and collision assets.
*/
void initializeAssets(CoreResources *core) {
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Player"]}, "Quad_Player");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_BasicEnemy"]}, "Quad_BasicEnemy");
    core->glenv.addQuad(QuadInfo{glm::vec3(0.0f), glm::vec3(32.0f, 32.0f, 0.0f), glm::vec4(1.0f), core->animations["Animation_Slash"]}, "Quad_Slash");

    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Player"]}, "EntityCollider_Player");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(16.0f, 16.0f, 16.0f), core->filters["Filter_Enemy"]}, "EntityCollider_Enemy");
    core->collisionspace.addCollider(EntityColliderInfo{glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), core->filters["Filter_Player"]}, "EntityCollider_Hitbox");

    core->executor.addEntityScript(EntityScriptInfo{&core->provider_ES_Player, nullptr, nullptr}, "ES_Player");
    core->executor.addEntityScript(EntityScriptInfo{&core->globalresources.provider_ES_Chaser, nullptr, nullptr}, "ES_Chaser");
    core->executor.addEntityScript(EntityScriptInfo{&core->globalresources.provider_ES_Lifetime, nullptr, nullptr}, "ES_Lifetime");
    
    core->entitymanager.addEntity(EntityInfo{"Group_Player", "ES_Player", 0, true, {"Quad_Player"}, {"EntityCollider_Player"}}, "Entity_Player");
    core->entitymanager.addEntity(EntityInfo{"Group_Enemy", "ES_Chaser", 0, true, {"Quad_BasicEnemy"}, {"EntityCollider_Enemy"}}, "Entity_BasicEnemy");
    core->entitymanager.addEntity(EntityInfo{"Group_Hitbox", "ES_Lifetime", 0, true, {}, {"EntityCollider_Hitbox"}}, "Entity_Hitbox");
    core->entitymanager.addEntity(EntityInfo{"Group_Effect", "ES_Lifetime", 0, true, {"Quad_Slash"}, {}}, "Entity_Slash");
}