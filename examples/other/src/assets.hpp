#ifndef ASSETS_HPP_
#define ASSETS_HPP_

#include "..\..\..\core\include\animation.hpp"
#include "..\..\..\core\include\filter.hpp"

/* Loads animations and returns a map using their names. */
std::unordered_map<std::string, Animation> loadAnimations();

/* Loads filters and returns a map using their names. */
std::unordered_map<std::string, Filter> loadFilters();

#endif