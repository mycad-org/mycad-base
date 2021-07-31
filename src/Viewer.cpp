#include "mycad/Viewer.h"

#include <iostream>

using namespace mycad;
GLFW_Application::GLFW_Application()
{
    init();
}

GLFW_Application::~GLFW_Application()
{
    glfwDestroyWindow(win);
    glfwTerminate();
}

GLFW_Application::operator bool() const
{
    return glfwInit() && win != nullptr;
}

void GLFW_Application::init()
{
    if(!glfwInit())
    {
        std::cerr << "There was an error initializing glfw" << '\n';
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    makeWindow();
}

void GLFW_Application::makeWindow()
{
    win = glfwCreateWindow(640, 480, "Hello, glfw!", NULL, NULL);
    if(win == nullptr)
    {
        std::cerr << "There was an error initializing the window" << '\n';
    }
}
