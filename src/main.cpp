// This is only a minimal viewer for mycad data structures. It is not (yet)
// intended to be a full CAD package

#include "mycad/Viewer.h"

#include <iostream>

int main()
{
    // Simple RAII, glfwTerminate will _always_ be called
    if(auto app = mycad::GLFW_Application())
    {
    }
}
