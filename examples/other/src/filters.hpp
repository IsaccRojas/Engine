#ifndef FILTERS_HPP_
#define FILTERS_HPP_

#include "..\..\..\core\include\filter.hpp"

/* Loads filters and returns a map using their names. */
std::unordered_map<std::string, Filter> loadFilters();

#endif