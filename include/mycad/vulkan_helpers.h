#ifndef MYCAD_VULKAN_HELPERS_HEADER
#define MYCAD_VULKAN_HELPERS_HEADER
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <cstdint>
#include <set>
#include <vector>

const int MAX_FRAMES_IN_FLIGHT = 2;

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
    vk::raii::SwapchainKHR swapchain;
    std::vector<VkImage> images;
    vk::Extent2D extent;
    vk::SurfaceFormatKHR format;
    std::vector<vk::raii::ImageView> views;
};

struct Renderer
{
    /* ~Renderer() */
    /* { */
    /*     renderTarget.device.waitIdle(); */
    /* } */

    void draw(vk::raii::Device const & device, int currentFrame);

    /* RenderTarget renderTarget; */
    vk::raii::Queue graphicsQueue;
    vk::raii::Queue presentQueue;
    SwapchainData scd;
    vk::raii::Pipeline pipeline;
    vk::raii::RenderPass renderPass;
    std::vector<vk::raii::Framebuffer> framebuffers;
    vk::raii::CommandPool commandPool;
    vk::raii::CommandBuffers commandBuffers;
    std::vector<vk::raii::Semaphore> imageAvailableSems;
    std::vector<vk::raii::Semaphore> renderFinishedSems;
    std::vector<vk::raii::Fence> inFlightFences;
    std::vector<std::optional<std::size_t>> imagesInFlight;
};

vk::raii::Instance makeInstance(ApplicationData const & app);

ChosenPhysicalDevice choosePhysicalDevice(
    vk::raii::Instance const & instance,
    ApplicationData const & app);

vk::raii::Device makeLogicalDevice(ChosenPhysicalDevice const & cpd);

SwapchainData makeSwapchain(
    ApplicationData const & app,
    ChosenPhysicalDevice const & cpd,
    vk::raii::Device const & device);

std::pair<vk::raii::Pipeline, vk::raii::RenderPass> makePipeline(
    vk::raii::Device const & device,
    SwapchainData const & scd);

std::vector<vk::raii::Framebuffer> makeFramebuffers(
    vk::raii::Device const & device,
    vk::raii::RenderPass const & renderPass,
    SwapchainData const &scd);

vk::raii::CommandPool makeCommandPool(
    vk::raii::Device const & device,
    uint32_t graphicsFamilyQueueIndex);

vk::raii::CommandBuffers makeCommandBuffers(
    vk::raii::Device const & device,
    vk::raii::CommandPool const & commandPool,
    std::size_t nBuffers);

Renderer makeRenderer(
    vk::raii::Device const & device,
    ApplicationData const & app,
    ChosenPhysicalDevice const & cpd);

void recordDrawCommands (Renderer const & rdr);

#endif // MYCAD_VULKAN_HELPERS_HEADER
