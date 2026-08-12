#include "filters.hpp"

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