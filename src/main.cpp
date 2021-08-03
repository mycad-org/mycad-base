// This is only a minimal viewer for mycad data structures. It is not (yet)
// intended to be a full CAD package

#include "mycad/GLFW_Application.h"
#include "mycad/GL_Renderer.h"

#include <iostream>

int main()
{
    // Simple RAII, glfwTerminate will _always_ be called
    if(auto app = mycad::GLFW_Application())
    {
        mycad::Renderer renderer;
        while(!glfwWindowShouldClose(app.win))
        {
            renderer.draw(app.win);
            glfwPollEvents();
        }
    }
}
