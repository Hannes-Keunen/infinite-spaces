#include "Input.h"

#include "Engine.h"
#include "GameHeader.h"

#include <cstring>
#include <memory>

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->key_press[key] = (action == GLFW_PRESS ? true : false);
    input->key[key] = (action == GLFW_RELEASE ? false : true);
}

void Input::ButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->mouse_button_press[button] = (action == GLFW_PRESS ? true : false);
    input->mouse_button[button] = (action == GLFW_RELEASE ? false : true);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // printf("Pick\n");
        GH_ENGINE->PickMouse();
    }
}

void Input::CursorCallback(GLFWwindow* window, double x, double y)
{
    auto input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    input->mouse_dx = x - input->mouse_x;
    input->mouse_dy = y - input->mouse_y;
    input->mouse_x = x;
    input->mouse_y = y;
}

Input::Input()
{
    memset(this, 0, sizeof(Input));
}

void Input::SetupCallbacks(GLFWwindow* window)
{
    glfwSetWindowUserPointer(window, this);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, ButtonCallback);
    glfwSetCursorPosCallback(window, CursorCallback);
}

void Input::EndFrame()
{
    memset(key_press, 0, sizeof(key_press));
    memset(mouse_button_press, 0, sizeof(mouse_button_press));
    mouse_dx = 0.0;
    mouse_dy = 0.0;
}

void Input::Update()
{
    glfwPollEvents();
}
