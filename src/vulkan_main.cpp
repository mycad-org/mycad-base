#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <iostream>
#include <vector>

const int WIDTH=800;
const int HEIGHT=600;

std::vector<const char *> validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

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
                .applicationVersion  = 1,
                .pEngineName         = "No Engine",
                .engineVersion       = 1,
                .apiVersion          = VK_API_VERSION_1_1
        };


        // Get required extensions to pass to vk::InstanceCreateInfo
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // check to make sure required validation layers are present
        std::vector<const char *> exts(glfwExtensions, glfwExtensions + glfwExtensionCount);
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Check to make sure requested validation layers are present
        std::vector<vk::LayerProperties> properties = vk::enumerateInstanceLayerProperties();

        for (auto const& layer : validationLayers)
        {
            auto hasName = [&layer](vk::LayerProperties const & prop)
            {
                return strcmp(layer, prop.layerName) == 0;
            };

            if (std::ranges::find_if(properties, hasName) == properties.end())
            {
                std::cout << "The layer \"" << layer
                          <<"\" was not found" << '\n';
                return 1;
            }
        }

        // Set up the debugging messenger CreateInfo
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
            .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                             | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                         | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                         | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            .pfnUserCallback = debugCallback
        };

        // initialize the vk::InstanceCreateInfo
        vk::InstanceCreateInfo instanceCreateInfo{
            .pNext                   = &debugCreateInfo,
            .pApplicationInfo        = &applicationInfo,
            .enabledLayerCount       = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames     = validationLayers.data(),
            .enabledExtensionCount   = static_cast<uint32_t>(exts.size()),
            .ppEnabledExtensionNames = exts.data()
        };

        // create an Instance
        vk::raii::Instance instance( context, instanceCreateInfo );

        // Create a "screen surface" to render to.
        VkSurfaceKHR rawSurface;
        if (glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface) != VK_SUCCESS)
        {
            std::cerr << "Error creating a vulkan surface" << std::endl;
            return 1;
        }
        // TODO: do we need to worry about this being destructed before
        // vk::raii::Instance?
        vk::raii::SurfaceKHR surface(instance, rawSurface);

        // set up the debug messenger. throws exception on failure I guess...
        vk::raii::DebugUtilsMessengerEXT dbgMessenger(instance, debugCreateInfo);

        // List available extensions
        std::cout << "Available vulkan extensions: " << '\n';
        for (const auto& extension : vk::enumerateInstanceExtensionProperties())
        {
            std::cout << "    " << extension.extensionName << '\n';
        }

        // Set up appropriate device
        auto devices = vk::raii::PhysicalDevices(instance);
        std::size_t whichDevice = 0;
        uint32_t whichQueueFamily = 0;
        bool foundGraphicsQueue = false;
        bool foundSurfaceQueue  = false;
        for (; whichDevice < devices.size(); whichDevice++)
        {
            // Reset in case both were not found last time
            foundGraphicsQueue = false;
            foundSurfaceQueue  = false;

            auto &device = devices.at(whichDevice);
            auto queues = device.getQueueFamilyProperties();
            for(; whichQueueFamily < queues.size(); whichQueueFamily++)
            {
                auto const &queue = queues.at(whichQueueFamily);
                if (not foundGraphicsQueue &&
                    queue.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    foundGraphicsQueue = true;
                }
                if (not foundSurfaceQueue &&
                    device.getSurfaceSupportKHR(whichQueueFamily, *surface))
                {
                    foundSurfaceQueue = true;
                }

                if (foundGraphicsQueue && foundSurfaceQueue)
                {
                    break;
                }
            }

            if (foundGraphicsQueue && foundSurfaceQueue)
            {
                break;
            }
        }

        if(not foundGraphicsQueue)
        {
            std::cerr << "Unable to find a Device with graphics support" << std::endl;
            return 1;
        }

        if(not foundSurfaceQueue)
        {
            std::cerr << "Unable to find a Device with support for the appropriate surface queue" << std::endl;
            return 1;
        }

        // Set up the logical device
        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo deviceQueueInfo{
            .queueFamilyIndex = whichQueueFamily,
            .queueCount       = 1,
            .pQueuePriorities = &queuePriority
        };

        vk::PhysicalDeviceFeatures deviceFeatures{};

        vk::DeviceCreateInfo deviceInfo{
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos    = &deviceQueueInfo,
            // not needed by newer vulkan implementations, but I guess leave for now
            .enabledLayerCount       = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames     = validationLayers.data(),
            // --- end "not needed" ----
            .pEnabledFeatures     = &deviceFeatures
        };

        // I guess these throws if it fails
        vk::raii::Device device(devices.at(whichDevice), deviceInfo);
        vk::raii::Queue queue(device, whichQueueFamily, 0);

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
