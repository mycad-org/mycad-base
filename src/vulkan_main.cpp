#include "mycad/vulkan_helpers.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

int main()
{
    ApplicationData app;

    Renderer rdr(app);

    int currentFrame = 0;
    int i = 0;
    while(!glfwWindowShouldClose(app.window) && i < 10)
    {
        rdr.draw(currentFrame);
        currentFrame = currentFrame == MAX_FRAMES_IN_FLIGHT - 1 ? 0 : currentFrame + 1;

        glfwPollEvents();
        i++;
    }
}
