#ifndef GLINIT_HPP_
#define GLINIT_HPP_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

/* Initializes OpenGL and creates and returns a window, using GLFW and GLEW.
   The window's context is set to be the current OpenGL context. Debug callback
   handlers are also set up.
   width - window width
   height - window height
   title - window title
   debug - whether to enable debug output or not
*/
GLFWwindow *glinit(int windowwidth, int windowheight, const char *title, bool debug);

#endif