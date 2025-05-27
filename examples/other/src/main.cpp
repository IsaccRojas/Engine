#include "coreinit.hpp"
#include "loop.hpp"

int main() {
    CoreResources core;
    initializeCore(&core);

    loop(&core);
    
    return 0;
}