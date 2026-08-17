#include "init.hpp"

const unsigned MAX_QUADS = 2048;

const unsigned WINDOW_WIDTH = 512;
const unsigned WINDOW_HEIGHT = 512;
const unsigned VIEW_PIXEL_WIDTH = WINDOW_WIDTH / 2;
const unsigned VIEW_PIXEL_HEIGHT = WINDOW_HEIGHT / 2;
const unsigned PIXEL_LEVELS = 16;

// tile 0, 0 would be located above and to the right of this position
const unsigned COORD_WIDTH = 15;
const unsigned COORD_HEIGHT = 13;
const int COORD_ORIGIN_PIXEL_X = 0;
const int COORD_ORIGIN_PIXEL_Y = 0;

const unsigned TEX_SPACE_WIDTH = 224;
const unsigned TEX_SPACE_HEIGHT = 80;
const unsigned TEX_SPACE_LEVELS = 2;

const unsigned UNIT_PIXEL_WIDTH = 16;
const unsigned UNIT_PIXEL_HEIGHT = 16;

void initialize() {
    srand(time(NULL));

    // get animation and filter maps
    globalstate.animations = loadAnimations();
    globalstate.filters = loadFilters();
    loadAssets();

    // initialize GLFW, OpenGL, and GLFWInput
    globalstate.glfwstate.init(WINDOW_WIDTH, WINDOW_HEIGHT, "title", true);
    globalstate.input.setWindow(globalstate.glfwstate.getWindowHandle(), VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT);
    globalstate.glenv.init(MAX_QUADS);
    globalstate.glenv.setTexArray(TEX_SPACE_WIDTH, TEX_SPACE_HEIGHT, TEX_SPACE_LEVELS);
    globalstate.glenv.setTexture(Image("gfx/sprites.png"), 0, 0, 0);
    globalstate.glenv.setTexture(Image("gfx/tiles.png"), 0, 0, 1);
    globalstate.glenv.setViewTopDown((UNIT_PIXEL_WIDTH * 7) + (UNIT_PIXEL_WIDTH / 2.0f), (UNIT_PIXEL_HEIGHT * 6) + (UNIT_PIXEL_HEIGHT / 2.0f), 1.0f);
    globalstate.glenv.setProjOrthographic(VIEW_PIXEL_WIDTH, VIEW_PIXEL_HEIGHT, float(PIXEL_LEVELS));

    // initialize map
    for (unsigned x = 0; x < COORD_WIDTH; x++) {
        globalstate.map.push_back(std::vector<TileInfo>());
        for (unsigned y = 0; y < COORD_HEIGHT; y++)
            globalstate.map.back().push_back(TileInfo{-1, -1});
    }
    
    globalstate.mapinfo = MapInfo{
        glm::vec2(UNIT_PIXEL_WIDTH, UNIT_PIXEL_HEIGHT),
        glm::vec2(COORD_WIDTH, COORD_HEIGHT),
        glm::vec2(COORD_ORIGIN_PIXEL_X, COORD_ORIGIN_PIXEL_Y)
    };

    globalstate.inventory["key"] = 0;
}