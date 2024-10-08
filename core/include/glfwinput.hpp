#ifndef INPUT_HPP_
#define INPUT_HPP_

#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <iostream>

#define PI_INPUT 3.14159265358979323846264338327950288

class GLFWInput {
    GLFWwindow *_win_h;
    int _win_width;
    int _win_height;
    int _pixel_width;
    int _pixel_height;
    bool _w_p;
    bool _a_p;
    bool _s_p;
    bool _d_p;
    bool _up_p;
    bool _left_p;
    bool _down_p;
    bool _right_p;
    bool _space_p;
    bool _enter_p;
    bool _tab_p;
    bool _esc_p;
    bool _m1_p;
    bool _m2_p;
    double _win_mouse_x;
    double _win_mouse_y;
    double _pixel_mouse_x;
    double _pixel_mouse_y;
public:
    GLFWInput(GLFWwindow *window, int pixelwidth, int pixelheight);
    GLFWInput();
    ~GLFWInput();

    void setWindow(GLFWwindow *window, int pixelwidth, int pixelheight);

    // default copy assignment/construction are fine (reference is read only)

    /* Updates contained input state based on current inputs being made at time of this call. */
    void update();

    /* Sets if internal state of a key should remain true until polled again (via update()). NOTE: Currently
       does not actually impact how this class works, as it will store the state anyway and only unset a
       key when it is released.
    */
    void setsticky(bool value);
    
    bool get_w();
    bool get_a();
    bool get_s();
    bool get_d();
    bool get_up();
    bool get_left();
    bool get_down();
    bool get_right();
    bool get_space();
    bool get_enter();
    bool get_tab();
    bool get_esc();
    bool get_m1();
    bool get_m2();

    /* Returns the input of WASD as a unit vector (up is in the positive y direction, right is in the positive x direction). */
    glm::vec2 inputdir();
    
    /* Returns the current mouse position within the window, scaled to the provided pixel dimensions, with the origin at the center. */
    glm::vec2 mousepos();
};

#endif