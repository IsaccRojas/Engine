#ifndef GLFWSTATE_HPP_
#define GLFWSTATE_HPP_

#include <GLFW/glfw3.h>
#include <iostream>

/* class GLErrorException
   This exception is thrown when an error is caught by the GLFW debug error handler.
*/
class GLFWErrorException : public std::runtime_error {
public:
   GLFWErrorException();
};

/* class GLFWState
   Wraps the GLFW library state. Currently only supports one window being initialized.
*/
class GLFWState {
   GLFWwindow *_win_h;
   int _width;
   int _height;
   const char *_title;
public:
   GLFWState(int width, int height, const char *title, bool debug);
   GLFWState();
   ~GLFWState();
   GLFWState(const GLFWState &other) = delete;
   GLFWState &operator=(const GLFWState &other) = delete;

   /* Initializes GLFW and instantiates a GLFWwindow, set to contained reference. */
   void init(int width, int height, const char *title, bool debug);
   void uninit();

   GLFWwindow *getWindowHandle();
};

#endif