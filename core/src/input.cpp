#include "../include/input.hpp"

Input::Input(GLFWwindow *window, int pixelwidth, int pixelheight) : _win_h(window), _pixel_width(pixelwidth), _pixel_height(pixelheight) {
    glfwGetWindowSize(_win_h, &_win_width, &_win_height);
}
Input::~Input() {}

void Input::update() {
    int state = glfwGetKey(_win_h, GLFW_KEY_W);
    if (state == GLFW_PRESS)
        _w_p = true;
    else if (state == GLFW_RELEASE)
        _w_p = false;
    
    state = glfwGetKey(_win_h, GLFW_KEY_A);
    if (state == GLFW_PRESS)
        _a_p = true;
    else if (state == GLFW_RELEASE)
        _a_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_S);
    if (state == GLFW_PRESS)
        _s_p = true;
    else if (state == GLFW_RELEASE)
        _s_p = false;
    
    state = glfwGetKey(_win_h, GLFW_KEY_D);
    if (state == GLFW_PRESS)
        _d_p = true;
    else if (state == GLFW_RELEASE)
        _d_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_UP);
    if (state == GLFW_PRESS)
        _up_p = true;
    else if (state == GLFW_RELEASE)
        _up_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_LEFT);
    if (state == GLFW_PRESS)
        _left_p = true;
    else if (state == GLFW_RELEASE)
        _left_p = false;
    
    state = glfwGetKey(_win_h, GLFW_KEY_DOWN);
    if (state == GLFW_PRESS)
        _down_p = true;
    else if (state == GLFW_RELEASE)
        _down_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_RIGHT);
    if (state == GLFW_PRESS)
        _right_p = true;
    else if (state == GLFW_RELEASE)
        _right_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS)
        _space_p = true;
    else if (state == GLFW_RELEASE)
        _space_p = false;

    state = glfwGetKey(_win_h, GLFW_KEY_ENTER);
    if (state == GLFW_PRESS)
        _enter_p = true;
    else if (state == GLFW_RELEASE)
        _enter_p = false;
    
    state = glfwGetKey(_win_h, GLFW_KEY_TAB);
    if (state == GLFW_PRESS)
        _tab_p = true;
    else if (state == GLFW_RELEASE)
        _tab_p = false;

    state = glfwGetMouseButton(_win_h, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
        _m1_p = true;
    else if (state == GLFW_RELEASE)
        _m1_p = false;

    state = glfwGetMouseButton(_win_h, GLFW_MOUSE_BUTTON_RIGHT);
    if (state == GLFW_PRESS)
        _m2_p = true;
    else if (state == GLFW_RELEASE)
        _m2_p = false;

    glfwGetCursorPos(_win_h, &_win_mouse_x, &_win_mouse_y);
    _pixel_mouse_x = (_pixel_width * (_win_mouse_x / _win_width)) - (_pixel_width / 2.0f);
    _pixel_mouse_y = (_pixel_height - (_pixel_height * (_win_mouse_y / _win_height))) - (_pixel_height / 2.0f);
};

void Input::setsticky(bool value) {
    if (value)
        glfwSetInputMode(_win_h, GLFW_STICKY_KEYS, GLFW_TRUE);
    else
        glfwSetInputMode(_win_h, GLFW_STICKY_KEYS, GLFW_FALSE);
}

bool Input::get_w() { return _w_p; }
bool Input::get_a() { return _a_p; }
bool Input::get_s() { return _s_p; }
bool Input::get_d() { return _d_p; }
bool Input::get_up() { return _up_p; }
bool Input::get_left() { return _left_p; }
bool Input::get_down() { return _down_p; }
bool Input::get_right() { return _right_p; }
bool Input::get_space() { return _space_p; }
bool Input::get_enter() { return _enter_p; }
bool Input::get_tab() { return _tab_p; }
bool Input::get_m1() { return _m1_p; }
bool Input::get_m2() { return _m2_p; }

glm::vec2 Input::inputdir() {
    float vertical = float(_w_p | _up_p) + (-1.0f * float(_s_p | _down_p));
    float horizontal = float(_d_p | _right_p) + (-1.0f * float(_a_p | _left_p));
    float angle = (horizontal != 0) ? glm::atan(glm::abs(vertical) / glm::abs(horizontal)) : (PI_INPUT / 2.0f);
    return glm::vec2(
        horizontal * glm::cos(angle),
        vertical * glm::sin(angle)
    );
}

glm::vec2 Input::mousepos() {
    return glm::vec2(_pixel_mouse_x, _pixel_mouse_y);
}