#ifndef MYCAD_VULKAN_HELPERS_HEADER
#define MYCAD_VULKAN_HELPERS_HEADER

#include "mycad/render_helpers.h"
#include "mycad/vulkan_types.h"

#include <cstdint>
#include <memory>
#include <vector>

struct ApplicationData
{
    ApplicationData();
    ~ApplicationData();

    GLFWwindow* window = nullptr;
};

struct ChosenPhysicalDevice
{
    ChosenPhysicalDevice(vk::raii::Instance const & instance, GLFWwindow * win);

    uptrPhysicalDevice physicalDevice;
    uptrSurfaceKHR surface;
    std::vector<uint32_t> queueIndices;
    uint32_t graphicsFamilyQueueIndex = 0;
    uint32_t presentFamilyQueueIndex = 0;
    uint32_t transferFamilyQueueIndex = 0;
};

struct SwapchainData
{
    SwapchainData(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device);

    uptrSwapchain swapchain;
    Images images;
    vk::Extent2D extent;
    vk::SurfaceFormatKHR surfaceFormat;
    ImageViews views;
};

class PipelineData
{
    public:
        PipelineData(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device);
        void rebuild(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device);

        std::unique_ptr<SwapchainData> scd;
        Framebuffers framebuffers;
        uptrPipeline pipeline;
        uptrPipelineLayout pipelineLayout;
        uptrRenderPass renderPass;
        uptrCommandPool commandPool;
        uptrCommandBuffers commandBuffers;
        uptrCommandPool transferCommandPool;

        uptrDescriptorLayout descriptorLayout;
        uptrDescriptorPool descriptorPool;
        uptrDescriptorSets descriptorSets;

        Buffers uniformBuffers;
        Memories uniformMemories;

        uptrImage textureImage;
        uptrImageView textureImageView;
        uptrMemory textureImageMemory;
        uptrSampler textureSampler;

        uptrImage depthImage;
        uptrMemory depthImageMemory;
        uptrImageView depthImageView;
        vk::Format depthFormat;

        uptrQueue graphicsQueue;
        uptrQueue presentQueue;
        uptrQueue transferQueue;
    private:
        void setupDescriptors(vk::raii::Device const & device);
        void makeFramebuffers(vk::raii::Device const & device);
        void makeCommands(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd);
        void makeRenderPass(vk::raii::Device const & device);
        void makePipeline(vk::raii::Device const & device);
        void setupDepthBuffer(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd);
        void setupTextures(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd);
        void transitionImageLayout(vk::raii::Device const & device, vk::raii::Image const & img, vk::Format fmt, vk::ImageLayout oldLayout, vk::ImageLayout  newLayout);
};

class Renderer
{
    public:
        Renderer(GLFWwindow * win, int maxFrames);

        ~Renderer();

        Renderer (Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        void rebuildPipeline();
        void draw(int currentFrame);

    private:
        void makeInstance();
        void makeLogicalDevice();
        void makeFramebuffers();
        void makePipelineAndRenderpass();
        void setupBuffers();
        void recordDrawCommands();

        // I know this is a whole mess of member variables, but honestly I don't
        // think there is any 'cleaner' way to do this. Vulkan is _very_
        // explicit, and the only alternative I could think of was to just make
        // a very deep hierarchy of shallow wrapper classes, but that seems
        // pointless
        bool framebufferResized = false;
        GLFWwindow* window = nullptr;
        vk::raii::Context context{};
        uptrInstance instance;
        std::unique_ptr<ChosenPhysicalDevice> cpd;
        uptrDevice device;
        std::unique_ptr<PipelineData> pld;
        Semaphores imageAvailableSems;
        Semaphores renderFinishedSems;
        Fences inFlightFences;
        MaybeIndices imagesInFlight;
        uptrBuffer vertexBuffer;
        uptrMemory vertexBufferMemory;
        uptrBuffer indexBuffer;
        uptrMemory indexBufferMemory;
        MVPBufferObject mvpMatrix;

        const std::vector<Vertex> vertices = {
            {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
        };

        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4
        };
};

#endif // MYCAD_VULKAN_HELPERS_HEADER
