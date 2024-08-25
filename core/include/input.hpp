#pragma once

#ifndef INPUT_HPP_
#define INPUT_HPP_

#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <iostream>

#define PI 3.14159265358979323846264338327950288

class Input {
    GLFWwindow * const _win_h;
    int _win_width;
    int _win_height;
    int _pixel_width;
    int _pixel_height;
    bool _w_p;
    bool _a_p;
    bool _s_p;
    bool _d_p;
    bool _m1_p;
    bool _m2_p;
    double _win_mouse_x;
    double _win_mouse_y;
    double _pixel_mouse_x;
    double _pixel_mouse_y;
public:
    Input(GLFWwindow *window, int pixelwidth, int pixelheight);
    ~Input();
    void update();
    void setsticky(bool value);
    bool get_w();
    bool get_a();
    bool get_s();
    bool get_d();
    bool get_m1();
    bool get_m2();
    /* Returns the input of WASD as a unit vector (up is in the positive y direction, right is in the positive x direction). */
    glm::vec2 inputdir();
    glm::vec2 mousepos();
};

#endif