#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>

class Input
{
public:
    Input();
    void SetupCallbacks(GLFWwindow* window);

    void EndFrame();
    void Update();

    // Keyboard
    bool key[GLFW_KEY_LAST];
    bool key_press[GLFW_KEY_LAST];

    // Mouse
    bool   mouse_button[GLFW_MOUSE_BUTTON_LAST];
    bool   mouse_button_press[GLFW_MOUSE_BUTTON_LAST];
    double mouse_x;
    double mouse_y;
    double mouse_dx;
    double mouse_dy;

    // Joystick
    // TODO:

    // Bindings
    // TODO:

    // Calibration
    // TODO:
private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void ButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorCallback(GLFWwindow* window, double x, double y);
};
