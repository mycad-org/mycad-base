#include "mycad/GLFW_Application.h"

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
    return valid;
}

void GLFW_Application::init()
{
    std::cout << "GLFW info: " << glfwGetVersionString() << '\n';

    if(!glfwInit())
    {
        std::cerr << "There was an error initializing glfw" << '\n';
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    win = glfwCreateWindow(640, 480, "LearnOpenGL", NULL, NULL);
    if(win == nullptr)
    {
        std::cerr << "There was an error initializing the window" << '\n';
        return;
    }

    glfwMakeContextCurrent(win);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "There was an error initializing glew" << '\n';
        return;
    }

    valid = true;

    const unsigned char* renderer = glGetString(GL_RENDERER); // get renderer string
    const unsigned char* version = glGetString(GL_VERSION); // version as a string

    std::cout << "Renderer: " << renderer << '\n';
    std::cout << "OpenGL version: " << version << std::endl;

    // depth-test tells gl to NOT draw things that are obscured by something
    // closer
    /* glEnable(GL_DEPTH_TEST); */
    /* glDepthFunc(GL_LESS); */
}
