#include "glinit.hpp"
#include "loop.hpp"

int main() {
    // --- OpenGL and window setup ---

    std::cout << "Initializing OpenGL" << std::endl;
    GLFWwindow *win_h = glinit(400, 400, "title", false);

    // --- main loop ---

    loop(win_h);

    return 0;
}