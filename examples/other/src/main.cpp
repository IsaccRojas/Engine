#include "init.hpp"
#include "loop.hpp"

int main() {
    CoreResources core;
    initializeCore(&core);
    initializeAssets(&core);

    loop(&core);
    
    return 0;
}