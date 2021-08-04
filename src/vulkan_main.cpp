#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>

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

        // set up the debug messenger. throws exception on failure I guess...
        vk::raii::DebugUtilsMessengerEXT dbgMessenger(instance, debugCreateInfo);

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
