#include "glinit.hpp"
#include "loop.hpp"

int main() {
    // --- OpenGL and window setup ---

    std::cout << "Initializing OpenGL" << std::endl;
    GLFWwindow *win_h = glinit(512, 512, "title", false);

    // --- main loop ---

    loop(win_h);

    std::cout << "Ending program" << std::endl;
    
    return 0;
}