#ifndef ANIMATIONS_HPP_
#define ANIMATIONS_HPP_

#include "C:\dev\include\GL\glew.h"
#include "..\..\..\core\include\glenv.hpp"

/* Loads animations and returns a map using their names. */
std::unordered_map<std::string, Animation> loadAnimations();

#endif