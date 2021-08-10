#include "mycad/vulkan_helpers.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

int main()
{
    ApplicationData app;

    if (app.window == nullptr)
    {
        std::cerr << "Could not create a glfw Window" << std::endl;
        std::exit(1);
    }

    vk::raii::Instance instance = makeInstance(app);
    ChosenPhysicalDevice cpd = choosePhysicalDevice(instance, app);

    vk::raii::Device device = makeLogicalDevice(cpd);
    Renderer rdr = makeRenderer(device, app, cpd);
    recordDrawCommands(rdr);

    int currentFrame = 0;
    while(!glfwWindowShouldClose(app.window))
    {
        rdr.draw(device, currentFrame);
        currentFrame = currentFrame == MAX_FRAMES_IN_FLIGHT - 1 ? 0 : currentFrame + 1;

        glfwPollEvents();
    }
}
