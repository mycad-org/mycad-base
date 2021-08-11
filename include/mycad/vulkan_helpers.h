#ifndef MYCAD_VULKAN_HELPERS_HEADER
#define MYCAD_VULKAN_HELPERS_HEADER
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

using uptrPhysicalDevice = std::unique_ptr<vk::raii::PhysicalDevice>;
using uptrSurfaceKHR     = std::unique_ptr<vk::raii::SurfaceKHR>;
using VulkanIndex        = uint32_t;
using uIndices           = std::set<VulkanIndex>;
using uptrSwapchain      = std::unique_ptr<vk::raii::SwapchainKHR>;
using Images             = std::vector<VkImage>;
using ImageViews         = std::vector<vk::raii::ImageView>;
using uptrDevice         = std::unique_ptr<vk::raii::Device>;
using uptrQueue          = std::unique_ptr<vk::raii::Queue>;
using uptrPipeline       = std::unique_ptr<vk::raii::Pipeline>;
using uptrRenderPass     = std::unique_ptr<vk::raii::RenderPass>;
using Framebuffers       = std::vector<vk::raii::Framebuffer>;
using uptrCommandPool    = std::unique_ptr<vk::raii::CommandPool>;
using uptrCommandBuffers = std::unique_ptr<vk::raii::CommandBuffers>;
using Semaphores         = std::vector<vk::raii::Semaphore>;
using Fences             = std::vector<vk::raii::Fence>;
using MaybeIndex         = std::optional<std::size_t>;
using MaybeIndices       = std::vector<MaybeIndex>;

struct ApplicationData
{
    ApplicationData();
    ~ApplicationData();

    GLFWwindow* window = nullptr;
    vk::raii::Context context{};
};

struct ChosenPhysicalDevice
{
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::SurfaceKHR surface;
    std::set<uint32_t> queueIndices;
    uint32_t graphicsFamilyQueueIndex;
    uint32_t presentFamilyQueueIndex;
};

struct SwapchainData
{
    SwapchainData(ApplicationData const & app, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device);

    uptrSwapchain swapchain;
    Images images;
    vk::Extent2D extent;
    vk::SurfaceFormatKHR surfaceFormat;
    ImageViews views;
};

class Renderer
{
    public:
        Renderer(vk::raii::Device const & device,
                 ApplicationData const & app,
                 ChosenPhysicalDevice const & cpd);

        /* ~Renderer() */
        /* { */
        /*     renderTarget.device.waitIdle(); */
        /* } */

        void draw(vk::raii::Device const & device, int currentFrame);

    private:
        void recordDrawCommands();

        // I know this is a whole mess of member variables, but honestly I don't
        // think there is any 'cleaner' way to do this. Vulkan is _very_
        // explicit, and the only alternative I could think of was to just make
        // a very deep hierarchy of shallow wrapper classes, but that seems
        // pointless
        uptrDevice device;
        uptrQueue graphicsQueue;
        uptrQueue presentQueue;
        std::unique_ptr<SwapchainData> scd;
        uptrPipeline pipeline;
        uptrRenderPass renderPass;
        Framebuffers framebuffers;
        uptrCommandPool commandPool;
        uptrCommandBuffers commandBuffers;
        Semaphores imageAvailableSems;
        Semaphores renderFinishedSems;
        Fences inFlightFences;
        MaybeIndices imagesInFlight;
};

vk::raii::Instance makeInstance(ApplicationData const & app);

ChosenPhysicalDevice choosePhysicalDevice(
    vk::raii::Instance const & instance,
    ApplicationData const & app);

vk::raii::Device makeLogicalDevice(ChosenPhysicalDevice const & cpd);

#endif // MYCAD_VULKAN_HELPERS_HEADER
