#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

#include <iostream>

const int WIDTH=800;
const int HEIGHT=600;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    // the very beginning: instantiate a context
    vk::raii::Context context;

    try
    {
        // initialize the vk::ApplicationInfo structure
        vk::ApplicationInfo applicationInfo{
                .pApplicationName    = "Hello Triangle",
                .applicationVersion = 1,
                .pEngineName         = "No Engine",
                .engineVersion       = 1,
                .apiVersion          = VK_API_VERSION_1_1
        };


        // Get required extensions to pass to vk::InstanceCreateInfo
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // initialize the vk::InstanceCreateInfo
        vk::InstanceCreateInfo instanceCreateInfo{
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = 0,
            .ppEnabledLayerNames     = nullptr,
            .enabledExtensionCount   = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions
        };

        // create an Instance
        vk::raii::Instance instance( context, instanceCreateInfo );

        std::cout << "Available vulkan extensions: " << '\n';
        for (const auto& extension : vk::enumerateInstanceExtensionProperties())
        {
            std::cout << "    " << extension.extensionName << '\n';
        }
    }
    catch ( vk::SystemError & err )
    {
        std::cout << "vk::SystemError: " << err.what() << std::endl;
        exit( -1 );
    }
    catch ( std::exception & err )
    {
        std::cout << "std::exception: " << err.what() << std::endl;
        exit( -1 );
    }
    catch ( ... )
    {
        std::cout << "unknown error\n";
        exit( -1 );
    }

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}
