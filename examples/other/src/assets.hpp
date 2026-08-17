#ifndef ASSETS_HPP_
#define ASSETS_HPP_

#include "implementations.hpp"

/* Loads animations and returns a map using their names. */
std::unordered_map<std::string, Animation> loadAnimations();

/* Loads filters and returns a map using their names. */
std::unordered_map<std::string, Filter> loadFilters();

/* Sets and maps assets for global state. */
void loadAssets();

#endif