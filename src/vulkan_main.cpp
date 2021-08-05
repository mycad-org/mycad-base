#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

const int WIDTH=800;
const int HEIGHT=600;

std::vector<const char *> const validationLayers =
{
    "VK_LAYER_KHRONOS_validation"
};

std::vector<const char *> const deviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
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
        std::set<uint32_t> whichQueues{};
        bool foundGraphicsQueue = false;
        bool foundSurfaceQueue  = false;
        uint32_t whichGraphicsFamily = 0;
        uint32_t whichSurfaceFamily  = 0;
        for (; whichDevice < devices.size(); whichDevice++)
        {
            // Check for required device extensions
            auto &device = devices.at(whichDevice);

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
            for(const auto& deviceExtension : device.enumerateDeviceExtensionProperties())
            {
                requiredExtensions.erase(deviceExtension.extensionName);
            }

            if(not requiredExtensions.empty())
            {
                continue;
            }

            // Ensure appropriate swap chain support
            /* auto surfaceCapabilities      = device.getSurfaceCapabilitiesKHR(*surface); */
            auto surfaceFormats      = device.getSurfaceFormatsKHR(*surface);
            auto surfacePresentModes = device.getSurfacePresentModesKHR(*surface);
            if (surfaceFormats.empty() || surfacePresentModes.empty())
            {
                continue;
            }


            // Reset in case both were not found last time
            foundGraphicsQueue  = false;
            foundSurfaceQueue   = false;
            whichGraphicsFamily = 0;
            whichSurfaceFamily  = 0;
            auto queues = device.getQueueFamilyProperties();
            for(auto const& queue : device.getQueueFamilyProperties())
            {
                if (not foundGraphicsQueue)
                {
                    if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
                    {
                        foundGraphicsQueue = true;
                    }

                    whichGraphicsFamily++;
                }
                if (not foundSurfaceQueue) 
                {
                    if (device.getSurfaceSupportKHR(whichSurfaceFamily, *surface))
                    {
                        foundSurfaceQueue = true;
                    }

                    whichSurfaceFamily++;
                }

                if (foundGraphicsQueue && foundSurfaceQueue)
                {
                    whichQueues = {whichGraphicsFamily, whichSurfaceFamily};
                    break;
                }
            }

            if (foundGraphicsQueue && foundSurfaceQueue)
            {
                break;
            }
        }

        if (whichGraphicsFamily == 0 && whichSurfaceFamily == 0 && not foundGraphicsQueue && not foundSurfaceQueue)
        {
            std::cerr << "Error finding device - it could be that the proper device extensions were not found" << std::endl;
            std::cerr << "    Also, it could be that the appropriate swap-chain support was not found." << std::endl;
            return 1;
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
        vk::PhysicalDeviceFeatures deviceFeatures{};

        float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for(uint32_t queueIndex : whichQueues)
        {
            vk::DeviceQueueCreateInfo ci{
                .queueFamilyIndex = queueIndex,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(ci);
        }

        vk::DeviceCreateInfo deviceInfo{
            .queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos       = queueCreateInfos.data(),
            // not needed by newer vulkan implementations, but I guess leave for now
            .enabledLayerCount       = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames     = validationLayers.data(),
            // --- end "not needed" ----
            .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames = deviceExtensions.data(),
            .pEnabledFeatures        = &deviceFeatures
        };

        // I guess these throws if it fails
        vk::raii::Device device(devices.at(whichDevice), deviceInfo);
        [[maybe_unused]] vk::raii::Queue graphicsQueue(device, whichGraphicsFamily, 0);
        [[maybe_unused]] vk::raii::Queue surfaceQueue(device, whichSurfaceFamily, 0);

        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
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

    glfwDestroyWindow(window);
    glfwTerminate();
}
