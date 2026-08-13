#include "assets.hpp"

using namespace glm;

std::unordered_map<std::string, Animation> loadAnimations() {
    std::unordered_map<std::string, Animation> animations;

    animations["Animation_BasicEnemy"] =
        Animation("Animation_BasicEnemy", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 16.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Key"] =
        Animation("Animation_Key", {
            Cycle("default", false, {
                Frame(vec3(16.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_LightBall"] =
        Animation("Animation_LightBall", {
            Cycle("default", true, {
                Frame(vec3(96.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(112.0f, 0.0f, 0.0f), vec2(16.0f), 6),
                Frame(vec3(128.0f, 0.0f, 0.0f), vec2(16.0f), 6)
            })
        });

    animations["Animation_Player"] =
        Animation("Animation_Player", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 0.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Stairs"] =
        Animation("Animation_Stairs", {
            Cycle("default", false, {
                Frame(vec3(32.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            }),
            Cycle("unlocked", false, {
                Frame(vec3(48.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            })
        });

    animations["Animation_Tile"] =
        Animation("Animation_Tile", {
            Cycle("default", false, {
                Frame(vec3(0.0f, 64.0f, 0.0f), vec2(16.0f), 0)
            }),
            Cycle("dark_floor", false, {
                Frame(vec3(160.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("light_floor", false, {
                Frame(vec3(176.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("stud", false, {
                Frame(vec3(192.0f, 16.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("brick", false, {
                Frame(vec3(208.0f, 16.0f, 1.0f), vec2(16.0f), 0)
            }),
            Cycle("air", false, {
                Frame(vec3(192.0f, 0.0f, 1.0f), vec2(16.0f), 0)
            }),
        });
    
    return animations;
}

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