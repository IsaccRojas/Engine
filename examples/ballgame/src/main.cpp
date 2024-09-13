#include "coreinit.hpp"
#include "loop.hpp"

int main() {
    // --- OpenGL and window setup ---

    std::cout << "Initializing core" << std::endl;
    CoreResources *core = new CoreResources();
    initializeCore(core);

    // --- main loop ---

    std::cout << "Calling loop" << std::endl;
    loop(core);

    std::cout << "Destroying core" << std::endl;
    delete core;

    std::cout << "Ending program" << std::endl;
    
    return 0;
}