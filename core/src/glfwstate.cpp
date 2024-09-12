#include "../include/glfwstate.hpp"

// glfw error handler
void glfw_err_handler(int code, const char* desc) {
    std::cout << "ERR: glfw_err_handler(): got error code " << code << ", description: " << desc << std::endl;
    throw GLFWErrorException();
}

GLFWErrorException::GLFWErrorException() : std::runtime_error("GLFW error") {}

GLFWState::GLFWState(int width, int height, const char *title, bool debug) {
    init(width, height, title, debug);
}
GLFWState::GLFWState() : _win_h(nullptr), _width(0), _height(0), _title(nullptr) {}
GLFWState::~GLFWState() {
    uninit();
}

void GLFWState::init(int width, int height, const char *title, bool debug) {
    if (_win_h)
        throw std::runtime_error("Attempt to initialize GLFWState with existing GLFWwindow");

    // set GLFW error callback, initialize GLFW, and set debug state on window to be created
    glfwSetErrorCallback(glfw_err_handler);
    glfwInit();
    if (debug)
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_FALSE);
    
    _win_h = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(_win_h);

    _width = width;
    _height = height;
    _title = title;
}

void GLFWState::uninit() {
    if (!_win_h)
        return;

    glfwDestroyWindow(_win_h);
    glfwTerminate();

    _win_h = nullptr;
    _width = 0;
    _height = 0;
    _title = nullptr;
}
GLFWwindow *GLFWState::getWindowHandle() { return _win_h; }