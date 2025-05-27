#include "../include/glfwinput.hpp"

bool GLFWInput::_checkKey(int jid) { return (GLFW_PRESS == glfwGetKey(_win_h, jid)); }

bool GLFWInput::_checkMouseButton(int jid) { return (GLFW_PRESS == glfwGetMouseButton(_win_h, jid)); }

GLFWInput::GLFWInput(GLFWwindow *window, int pixel_width, int pixel_height) {
    setWindow(window, pixel_width, pixel_height);
    reset();
}
GLFWInput::GLFWInput() : 
    _win_h(nullptr),
    _win_width(0),
    _win_height(0),
    _pixel_width(0), 
    _pixel_height(0)
{
    reset();
}
GLFWInput::~GLFWInput() { /* automatic destruction is fine */ }

void GLFWInput::setWindow(GLFWwindow *window, int pixel_width, int pixel_height) {
    _win_h = window;
    _pixel_width = pixel_width;
    _pixel_height = pixel_height;

    if (_win_h)
        glfwGetWindowSize(_win_h, &_win_width, &_win_height);
    else
        throw std::runtime_error("Attempt to get window size with invalid GLFWwindow reference");
}

void GLFWInput::reset() {
    _w_p = false;
    _a_p = false;
    _s_p = false;
    _d_p = false;
    _e_p = false;
    _up_p = false;
    _left_p = false;
    _down_p = false;
    _right_p = false;
    _space_p = false;
    _enter_p = false;
    _tab_p = false;
    _esc_p = false;
    _m1_p = false;
    _m2_p = false;

    _win_mouse_x = 0.0f;
    _win_mouse_y = 0.0f;
    _pixel_mouse_x = 0.0f;
    _pixel_mouse_y = 0.0f;
    
    _has_joystick = false;
    _leftbumper_p = false;
    _rightbumper_p = false;
    _start_p = false;
    _button_a_p = false;
    _leftstick_x = 0.0f;
    _leftstick_y = 0.0f;
    _rightstick_x = 0.0f;
    _rightstick_y = 0.0f;
}

void GLFWInput::update() {
    if (!_win_h)
        throw std::runtime_error("Attempt to call on Input instance with invalid GLFWwindow reference");

    _w_p = _checkKey(GLFW_KEY_W);
    _a_p = _checkKey(GLFW_KEY_A);
    _s_p = _checkKey(GLFW_KEY_S);
    _d_p = _checkKey(GLFW_KEY_D);
    _e_p = _checkKey(GLFW_KEY_E);
    _up_p = _checkKey(GLFW_KEY_UP);
    _left_p = _checkKey(GLFW_KEY_LEFT);
    _down_p = _checkKey(GLFW_KEY_DOWN);
    _right_p = _checkKey(GLFW_KEY_RIGHT);
    _space_p = _checkKey(GLFW_KEY_SPACE);
    _enter_p = _checkKey(GLFW_KEY_ENTER);
    _tab_p = _checkKey(GLFW_KEY_TAB);
    _esc_p = _checkKey(GLFW_KEY_ESCAPE);
    _m1_p = _checkMouseButton(GLFW_MOUSE_BUTTON_LEFT);
    _m2_p = _checkMouseButton(GLFW_MOUSE_BUTTON_RIGHT);

    glfwGetCursorPos(_win_h, &_win_mouse_x, &_win_mouse_y);
    _pixel_mouse_x = (_pixel_width * (_win_mouse_x / _win_width)) - (_pixel_width / 2.0f);
    _pixel_mouse_y = (_pixel_height - (_pixel_height * (_win_mouse_y / _win_height))) - (_pixel_height / 2.0f);

    _has_joystick = glfwJoystickIsGamepad(GLFW_JOYSTICK_1);
    if (_has_joystick) {
        glfwGetGamepadState(GLFW_JOYSTICK_1, &_state);
        _leftbumper_p = GLFW_PRESS == _state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
        _rightbumper_p = GLFW_PRESS == _state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER];
        _start_p = GLFW_PRESS == _state.buttons[GLFW_GAMEPAD_BUTTON_START];
        _button_a_p = GLFW_PRESS == _state.buttons[GLFW_GAMEPAD_BUTTON_A];
        _leftstick_x = _state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        _leftstick_y = -1.0f * _state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        _rightstick_x = _state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
        _rightstick_y = -1.0f * _state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
    } else {
        _leftbumper_p = false;
        _rightbumper_p = false;
        _leftstick_x = 0.0f;
        _leftstick_y = 0.0f;
        _rightstick_x = 0.0f;
        _rightstick_y = 0.0f;
    }
};

void GLFWInput::setsticky(bool value) {
    if (!_win_h)
        throw std::runtime_error("Attempt to call on Input instance with invalid GLFWwindow reference");
    
    if (value)
        glfwSetInputMode(_win_h, GLFW_STICKY_KEYS, GLFW_TRUE);
    else
        glfwSetInputMode(_win_h, GLFW_STICKY_KEYS, GLFW_FALSE);  
}

bool GLFWInput::get_w() { return _w_p; }
bool GLFWInput::get_a() { return _a_p; }
bool GLFWInput::get_s() { return _s_p; }
bool GLFWInput::get_d() { return _d_p; }
bool GLFWInput::get_e() { return _e_p; }
bool GLFWInput::get_up() { return _up_p; }
bool GLFWInput::get_left() { return _left_p; }
bool GLFWInput::get_down() { return _down_p; }
bool GLFWInput::get_right() { return _right_p; }
bool GLFWInput::get_space() { return _space_p; }
bool GLFWInput::get_enter() { return _enter_p; }
bool GLFWInput::get_tab() { return _tab_p; }
bool GLFWInput::get_esc() { return _esc_p; }
bool GLFWInput::get_m1() { return _m1_p; }
bool GLFWInput::get_m2() { return _m2_p; }
bool GLFWInput::get_leftbumper() { return _leftbumper_p; }
bool GLFWInput::get_rightbumper() { return _rightbumper_p; }
bool GLFWInput::get_start() { return _start_p; }
bool GLFWInput::get_button_a() { return _button_a_p; }

glm::vec2 GLFWInput::inputdir() {
    float vertical = float(_w_p | _up_p) + (-1.0f * float(_s_p | _down_p));
    float horizontal = float(_d_p | _right_p) + (-1.0f * float(_a_p | _left_p));
    float angle = (horizontal != 0) ? glm::atan(glm::abs(vertical) / glm::abs(horizontal)) : (PI_INPUT / 2.0f);
    return glm::vec2(
        horizontal * glm::cos(angle),
        vertical * glm::sin(angle)
    );
}

glm::vec2 GLFWInput::mousepos() {
    return glm::vec2(_pixel_mouse_x, _pixel_mouse_y);
}

bool GLFWInput::has_joystick() {
    return _has_joystick;
}

glm::vec2 GLFWInput::leftstick() {
    return glm::vec2(_leftstick_x, _leftstick_y);
}

glm::vec2 GLFWInput::rightstick() {
    return glm::vec2(_rightstick_x, _rightstick_y);
}