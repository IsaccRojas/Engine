#include "coreinit.hpp"

const char *ANIMATION_DIR = "./animconfig";
const char *FILTER_DIR = "./filterconfig";

const unsigned MAX_COUNT = 2048;
const unsigned EXECUTION_QUEUES = 2;

const unsigned WINDOW_WIDTH = 512;
const unsigned WINDOW_HEIGHT = 512;
const unsigned PIXEL_WIDTH = WINDOW_WIDTH / 2;
const unsigned PIXEL_HEIGHT = WINDOW_HEIGHT / 2;
const unsigned PIXEL_LEVELS = 16;

const unsigned TEX_SPACE_WIDTH = 133;
const unsigned TEX_SPACE_HEIGHT = 318;
const unsigned TEX_SPACE_LEVELS = 4;

const float CLEAR_COLOR_GRAY = 0.35f;

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

    // set up GLEnv
    std::cout << "Setting up GLEnv" << std::endl;
    core->glenv.init(MAX_COUNT);
    core->glenv.setTexArray(TEX_SPACE_WIDTH, TEX_SPACE_HEIGHT, TEX_SPACE_LEVELS);
    core->glenv.setTexture(Image("gfx/objects.png"), 0, 0, 0);
    core->glenv.setTexture(Image("gfx/effects.png"), 0, 0, 1);
    core->glenv.setTexture(Image("gfx/characters1.png"), 0, 0, 2); // each character is 7x24 pixels
    core->glenv.setTexture(Image("gfx/characters2.png"), 0, 0, 3); // each character is 5x10 pixels
    
    float halfwidth = float(PIXEL_WIDTH) / 2.0f;
    float halfheight = float(PIXEL_HEIGHT) / 2.0f;
    core->glenv.setView(glm::lookAt(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    core->glenv.setProj(glm::ortho(-1.0f * halfwidth, halfwidth, -1.0f * halfheight, halfheight, 0.0f, float(PIXEL_LEVELS)));
    core->glenv.setWindowSpace(WINDOW_WIDTH, WINDOW_HEIGHT);
    core->glenv.setPixelSpace(PIXEL_WIDTH, PIXEL_HEIGHT, PIXEL_LEVELS);

    // set up Executor
    std::cout << "Setting up Executor" << std::endl;
    core->executor.init(EXECUTION_QUEUES, &core->glenv, &core->animations, &core->box_space, &core->sphere_space, &core->filters);

    // set up GLFWInput
    std::cout << "Setting up GLFWInput" << std::endl;
    core->input.setWindow(core->state.getWindowHandle(), PIXEL_WIDTH, PIXEL_HEIGHT);

    std::cout << "Setting some OpenGL parameters" << std::endl;
    glfwSwapInterval(1);
    glClearColor(CLEAR_COLOR_GRAY, CLEAR_COLOR_GRAY, CLEAR_COLOR_GRAY, 0.0f);
    glEnable(GL_DEPTH_TEST);
}