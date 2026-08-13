#ifndef ASSETS_HPP_
#define ASSETS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "..\..\..\core\include\glfwstate.hpp"
#include "..\..\..\core\include\glfwinput.hpp"
#include "..\..\..\core\include\entity.hpp"

/* Loads animations and returns a map using their names. */
std::unordered_map<std::string, Animation> loadAnimations();

/* Loads filters and returns a map using their names. */
std::unordered_map<std::string, Filter> loadFilters();

#endif