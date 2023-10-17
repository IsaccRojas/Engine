#include "glinit.hpp"

// glfw error handler
void glfw_err_handler(int code, const char* desc) {
    std::cout 
        << "ERR: glfw_err_handler(): got error code " 
        << code
        << ", description: " 
        << desc
        << std::endl;
    exit(0);
}

// opengl error handler
void GLAPIENTRY gl_err_handler(
    GLenum src,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei len,
    const GLchar* msg,
    const void* param
) 
{
    std::cout << "ERR: gl_err_handler(): " << msg << std::endl;
    exit(0);
}

GLFWwindow *glinit(int windowwidth, int windowheight, const char *title, bool debug) {
    // set GLFW error callback, initialize GLFW, and enable debug output on context
    glfwSetErrorCallback(glfw_err_handler);
    glfwInit();
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    // create window
    GLFWwindow* win_h = glfwCreateWindow(windowwidth, windowheight, title, NULL, NULL);
    glfwMakeContextCurrent(win_h);

    // initialize glew
    glewInit();
    if (debug)
        glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_err_handler, NULL);

    return win_h;
}