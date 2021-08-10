#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include "shaders/triangle_colors_vshader.h"
#include "shaders/gradient_fshader.h"

#include <algorithm>
#include <iostream>
#include <limits>
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

const int MAX_FRAMES_IN_FLIGHT = 2;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/)
{

    std::cerr << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

struct ApplicationData
{
    ApplicationData(){
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

        // List available extensions
        std::cout << "Available vulkan extensions: " << '\n';
        for (const auto& extension : vk::enumerateInstanceExtensionProperties())
        {
            std::cout << "    " << extension.extensionName << '\n';
        }

    }

    ~ApplicationData()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    GLFWwindow* window = nullptr;
    vk::raii::Context context{};
};

vk::raii::Instance makeInstance(ApplicationData const & app)
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
            std::exit(1);
        }
    }

    // Set up the debugging messenger CreateInfo
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{
        .messageSeverity = //vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
                           vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
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

    // return an Instance
    return {app.context, instanceCreateInfo};
}

struct ChosenPhysicalDevice
{
    vk::raii::PhysicalDevice physicalDevice;
    vk::raii::SurfaceKHR surface;
    std::set<uint32_t> queueIndices;
    uint32_t graphicsFamilyQueueIndex;
    uint32_t presentFamilyQueueIndex;
};

ChosenPhysicalDevice choosePhysicalDevice(vk::raii::Instance const & instance, ApplicationData const & app)
{
    // Create a "screen surface" to render to.
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, app.window, nullptr, &rawSurface) != VK_SUCCESS)
    {
        std::cerr << "Error creating a vulkan surface" << std::endl;
        std::exit(1);
    }
    // TODO: do we need to worry about this being destructed before
    // vk::raii::Instance?
    vk::raii::SurfaceKHR surface(instance, rawSurface);

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
        std::cout << "Looking at device number: " << whichDevice << '\n';
        // Check for required device extensions
        auto &device = devices.at(whichDevice);

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for(const auto& deviceExtension : device.enumerateDeviceExtensionProperties())
        {
            requiredExtensions.erase(deviceExtension.extensionName);
        }

        if(not requiredExtensions.empty())
        {
            std::cout << "    bailing, all extensions not found" << '\n';
            continue;
        }

        // Ensure appropriate swap chain support
        auto surfaceFormats      = device.getSurfaceFormatsKHR(*surface);
        auto surfacePresentModes = device.getSurfacePresentModesKHR(*surface);
        if (surfaceFormats.empty() || surfacePresentModes.empty())
        {
            std::cout << "    bailing, swapchain support not found " << '\n';
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
            std::cout << "    Looking at queue number: " << whichSurfaceFamily << '\n';
            if (not foundGraphicsQueue)
            {
                if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    std::cout << "        found graphics bit, index = " << whichGraphicsFamily << '\n';
                    foundGraphicsQueue = true;
                }
                else
                {
                    whichGraphicsFamily++;
                }
            }
            if (not foundSurfaceQueue) 
            {
                if (device.getSurfaceSupportKHR(whichSurfaceFamily, *surface))
                {
                    std::cout << "        found surface support, index = " << whichSurfaceFamily << '\n';
                    foundSurfaceQueue = true;
                }
                else
                {
                    whichSurfaceFamily++;
                }
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

    std::cout << "whichGraphicsFamily = " << whichGraphicsFamily << '\n';
    std::cout << "whichSurfaceFamily = " << whichSurfaceFamily << '\n';

    if (whichGraphicsFamily == 0 && whichSurfaceFamily == 0 && not foundGraphicsQueue && not foundSurfaceQueue)
    {
        std::cerr << "Error finding device - it could be that the proper device extensions were not found" << std::endl;
        std::cerr << "    Also, it could be that the appropriate swap-chain support was not found." << std::endl;
        std::exit(1);
    }

    if(not foundGraphicsQueue)
    {
        std::cerr << "Unable to find a Device with graphics support" << std::endl;
        std::exit(1);
    }

    if(not foundSurfaceQueue)
    {
        std::cerr << "Unable to find a Device with support for the appropriate surface queue" << std::endl;
        std::exit(1);
    }

    ChosenPhysicalDevice cpd {
        .physicalDevice{instance, *devices.at(whichDevice)},
        .surface = std::move(surface),
        .queueIndices = whichQueues,
        .graphicsFamilyQueueIndex = whichGraphicsFamily,
        .presentFamilyQueueIndex = whichSurfaceFamily
    };

    return cpd;
}

vk::raii::Device makeLogicalDevice(ChosenPhysicalDevice const & cpd)
{
    // Set up the logical device
    vk::PhysicalDeviceFeatures deviceFeatures{};

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for(uint32_t queueIndex : cpd.queueIndices)
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

    return {cpd.physicalDevice, deviceInfo};
}

struct SwapchainData
{
    vk::raii::SwapchainKHR swapchain;
    std::vector<VkImage> images;
    vk::Extent2D extent;
    vk::SurfaceFormatKHR format;
    std::vector<vk::raii::ImageView> views;
};

SwapchainData makeSwapchain(ApplicationData const & app, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device)
{
    // create the swap chain
    auto surfaceFormats      = cpd.physicalDevice.getSurfaceFormatsKHR(*cpd.surface);
    auto surfacePresentModes = cpd.physicalDevice.getSurfacePresentModesKHR(*cpd.surface);
    auto surfaceCapabilities = cpd.physicalDevice.getSurfaceCapabilitiesKHR(*cpd.surface);

    vk::SurfaceFormatKHR surfaceFormat{
        .format     = vk::Format::eB8G8R8A8Srgb,
        .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
    };

    auto presentMode = vk::PresentModeKHR::eMailbox;

    vk::Extent2D extent;

    if (std::ranges::find(surfaceFormats, surfaceFormat) == surfaceFormats.end())
    {
        surfaceFormat = surfaceFormats.front();
    }

    if (std::ranges::find(surfacePresentModes, presentMode) == surfacePresentModes.end())
    {
        presentMode = vk::PresentModeKHR::eFifo;
    }

    if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        extent = surfaceCapabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(app.window, &width, &height);

        extent.width  = std::clamp(
                static_cast<uint32_t>(width),
                surfaceCapabilities.minImageExtent.width,
                surfaceCapabilities.maxImageExtent.width
                );
        extent.height  = std::clamp(
                static_cast<uint32_t>(width),
                surfaceCapabilities.minImageExtent.height,
                surfaceCapabilities.maxImageExtent.height
                );
    }

    // It's recommended to request one more than the minimum since otherwise
    // we "may have to wait on the driver to complete internal operations
    // before we can acquire another image to render to"
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    uint32_t maxImageCount = surfaceCapabilities.maxImageCount;
    if (maxImageCount > 0 && imageCount > maxImageCount)
    {
        imageCount = maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapchainInfo{
        .surface = *cpd.surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (cpd.graphicsFamilyQueueIndex != cpd.presentFamilyQueueIndex)
    {
        swapchainInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainInfo.queueFamilyIndexCount = 2;

        uint32_t familyIndices[] = {cpd.graphicsFamilyQueueIndex, cpd.presentFamilyQueueIndex};
        swapchainInfo.pQueueFamilyIndices = familyIndices;
    }
    else
    {
        swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    vk::raii::SwapchainKHR swapchain(device, swapchainInfo);

    // Create an image "view" for each swapchain image
    auto swapchainImages = swapchain.getImages();
    std::vector<vk::raii::ImageView> swapchainImageViews;
    for(const auto& swapchainImage : swapchainImages)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo{
            .image = swapchainImage,
            .viewType = vk::ImageViewType::e2D,
            .format = surfaceFormat.format,
            .components = {
                .r = vk::ComponentSwizzle::eIdentity,
                .g = vk::ComponentSwizzle::eIdentity,
                .b = vk::ComponentSwizzle::eIdentity,
                .a = vk::ComponentSwizzle::eIdentity
            },
            .subresourceRange = {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        swapchainImageViews.emplace_back(device, imageViewCreateInfo);
    }

    return {
        .swapchain = std::move(swapchain),
        .images = swapchainImages,
        .extent = extent,
        .format = surfaceFormat,
        .views = std::move(swapchainImageViews)
    };
}

std::pair<vk::raii::Pipeline, vk::raii::RenderPass> makePipeline(vk::raii::Device const & device, SwapchainData const & scd)
{
    // Attach shaders
    vk::ShaderModuleCreateInfo vShaderInfo{
        .codeSize = triangle_colors_vshader_len,
        .pCode = reinterpret_cast<const uint32_t*>(triangle_colors_vshader)
    };
    vk::ShaderModuleCreateInfo fShaderInfo{
        .codeSize = gradient_fshader_len,
        .pCode = reinterpret_cast<const uint32_t*>(gradient_fshader)
    };

    vk::raii::ShaderModule vShaderModule(device, vShaderInfo);
    vk::raii::ShaderModule fShaderModule(device, fShaderInfo);

    vk::PipelineShaderStageCreateInfo shaderStages[] = {
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *vShaderModule,
            .pName = "main"
        },
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *fShaderModule,
            .pName = "main"
        }
    };

    // Define "fixed" pipeline stages
    [[maybe_unused]] vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0,
    };

    [[maybe_unused]] vk::PipelineInputAssemblyStateCreateInfo inputAssyInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE
    };

    vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) scd.extent.width,
        .height = (float) scd.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vk::Rect2D scissor{
        .offset = {0, 0},
        .extent = scd.extent
    };

    vk::PipelineViewportStateCreateInfo viewportStateInfo {
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo{
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisamplingInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE
    };

    vk::PipelineColorBlendAttachmentState colorAttachmentState{
        .blendEnable = VK_FALSE,
        .colorWriteMask = vk::ColorComponentFlagBits::eR
                        | vk::ColorComponentFlagBits::eG
                        | vk::ColorComponentFlagBits::eB
                        | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlendingInfo{
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorAttachmentState
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{}; // no uniforms at the moment
    vk::raii::PipelineLayout pipelineLayout(device, pipelineLayoutInfo);

    vk::AttachmentDescription colorAttachment{
        .format = scd.format.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    vk::AttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    // Wait until render is complete: note, this could be accomplished
    // in semImageAvail but setting waitStages equal to
    // vk::PipeLineStageFlagsBit::eTopOfPipe . We're doing this approach
    // to learn how sub-pass dependencies can be managed
    vk::SubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = {},
        .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
    };

    vk::RenderPassCreateInfo renderPassInfo{
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    vk::raii::RenderPass renderPass(device, renderPassInfo);

    // Create the actual graphics pipeline!!!
    vk::GraphicsPipelineCreateInfo pipelineInfo{
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssyInfo,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizerInfo,
        .pMultisampleState = &multisamplingInfo,
        .pColorBlendState = &colorBlendingInfo,
        .layout = *pipelineLayout,
        .renderPass = *renderPass,
        .subpass = 0
    };

    return {vk::raii::Pipeline{device, nullptr, pipelineInfo}, std::move(renderPass)};
}

std::vector<vk::raii::Framebuffer> makeFramebuffers(vk::raii::Device const & device, vk::raii::RenderPass const & renderPass, SwapchainData const &scd)
{
    // create framebuffers
    std::vector<vk::raii::Framebuffer> swapchainFramebuffers;
    for(const auto& swapchainImageView : scd.views)
    {
        vk::FramebufferCreateInfo createInfo{
            .renderPass = *renderPass,
            .attachmentCount = 1,
            .pAttachments = &(*swapchainImageView),
            .width = scd.extent.width,
            .height = scd.extent.height,
            .layers = 1
        };

        swapchainFramebuffers.emplace_back(device, createInfo);
    }

    return swapchainFramebuffers;
}

vk::raii::CommandPool makeCommandPool(vk::raii::Device const & device, uint32_t graphicsFamilyQueueIndex)
{
    // Create the command pool
    vk::CommandPoolCreateInfo poolInfo{
        .queueFamilyIndex = graphicsFamilyQueueIndex
    };

    return {device, poolInfo};
}
vk::raii::CommandBuffers makeCommandBuffers(vk::raii::Device const & device, vk::raii::CommandPool const & commandPool, std::size_t nBuffers)
{
    // Create the command buffers
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(nBuffers)
    };

    return {device, allocateInfo};
}

struct Renderer
{
    SwapchainData scd;
    vk::raii::Pipeline pipeline;
    vk::raii::RenderPass renderPass;
    std::vector<vk::raii::Framebuffer> framebuffers;
};

Renderer makeRenderer(vk::raii::Device const & device, ApplicationData const & app, ChosenPhysicalDevice const & cpd)
{
    auto scd = makeSwapchain(app, cpd, device);
    auto [pipeline, renderPass] = makePipeline(device, scd);
    auto framebuffers = makeFramebuffers(device, renderPass, scd);

    return {
        .scd = std::move(scd),
        .pipeline = std::move(pipeline),
        .renderPass = std::move(renderPass),
        .framebuffers = std::move(framebuffers)
    };
}

int main()
{
    ApplicationData app;

    if (app.window == nullptr)
    {
        std::cerr << "Could not create a glfw Window" << std::endl;
        std::exit(1);
    }

    vk::raii::Instance instance = makeInstance(app);
    // set up the debug messenger. throws exception on failure I guess...
    /* vk::raii::DebugUtilsMessengerEXT dbgMessenger(instance, debugCreateInfo); */

    try
    {
        ChosenPhysicalDevice cpd = choosePhysicalDevice(instance, app);

        auto device = makeLogicalDevice(cpd);
        vk::raii::Queue graphicsQueue(device, cpd.graphicsFamilyQueueIndex, 0);
        vk::raii::Queue presentQueue(device, cpd.presentFamilyQueueIndex, 0);

        Renderer rdr = makeRenderer(device, app, cpd);

        vk::raii::CommandPool commandPool = makeCommandPool(device, cpd.graphicsFamilyQueueIndex);
        vk::raii::CommandBuffers commandBuffers = makeCommandBuffers(device, commandPool, rdr.framebuffers.size());

        // Record the commands
        for(std::size_t i = 0; i < commandBuffers.size(); i++)
        {
            vk::CommandBufferBeginInfo beginInfo{};

            const auto& commandBuffer = commandBuffers.at(i);

            //==== begin command
            commandBuffer.begin(beginInfo);

            vk::ClearValue clearColor = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};

            vk::RenderPassBeginInfo renderPassBeginInfo{
                .renderPass = *rdr.renderPass,
                .framebuffer = *rdr.framebuffers.at(i),
                .renderArea = {
                    .offset = {0, 0},
                    .extent = rdr.scd.extent
                },
                .clearValueCount = 1,
                .pClearValues = &clearColor
            };

            //======== begin render pass
            commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
            commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *rdr.pipeline);
            commandBuffer.draw(3, 1, 0, 0);
            commandBuffer.endRenderPass();
            //======== end render pass

            commandBuffer.end();
            //==== end command
        }

        // Semaphores to sync gpu stuff
        std::vector<vk::raii::Semaphore> imageAvailableSems;
        std::vector<vk::raii::Semaphore> renderFinishedSems;
        std::vector<vk::raii::Fence> inFlightFences;

        // Each image either has not been used yet, or has a inFlightFence that
        // it is associated with
        std::vector<std::optional<std::size_t>> imagesInFlight(rdr.scd.views.size(), std::nullopt);

        for([[maybe_unused]] int const i : std::ranges::iota_view(0, MAX_FRAMES_IN_FLIGHT))
        {
            imageAvailableSems.emplace_back(device, vk::SemaphoreCreateInfo());
            renderFinishedSems.emplace_back(device, vk::SemaphoreCreateInfo());

            vk::FenceCreateInfo fenceInfo{
                .flags = vk::FenceCreateFlagBits::eSignaled
            };
            inFlightFences.emplace_back(device, fenceInfo);
        }

        int currentFrame = 0;
        while(!glfwWindowShouldClose(app.window))
        {
            auto& frameFence      = *inFlightFences.at(currentFrame);
            // Wait for any previous frames that haven't finished yet
            [[maybe_unused]] auto waitResult = device.waitForFences({frameFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());

            // Acquire an image from the swap chain
            auto [res, imgIndex] = rdr.scd.swapchain.acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSems.at(currentFrame), nullptr);
            auto& maybeFenceIndex =  imagesInFlight.at(imgIndex);

            // Check if this image is still "in flight", and wait if so
            if(maybeFenceIndex.has_value())
            {
                std::size_t i = maybeFenceIndex.value();
                [[maybe_unused]] auto imageWaitRes = device.waitForFences(*inFlightFences.at(i), VK_TRUE, std::numeric_limits<uint64_t>::max());
            }

            imagesInFlight[imgIndex] = currentFrame;

            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

            // submit the command to the graphics queue
            vk::SubmitInfo submitInfo{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &(*imageAvailableSems.at(currentFrame)),
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &(*commandBuffers.at(imgIndex)),
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = &(*renderFinishedSems.at(currentFrame))
            };

            device.resetFences({frameFence});

            graphicsQueue.submit({submitInfo}, frameFence);

            // submit something or other to the "present" queue (this draws!)
            vk::PresentInfoKHR presentInfo{
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = &(*renderFinishedSems.at(currentFrame)),
                .swapchainCount = 1,
                .pSwapchains = &(*rdr.scd.swapchain),
                .pImageIndices = &imgIndex
            };

            [[maybe_unused]] auto presres = presentQueue.presentKHR(presentInfo);

            currentFrame = currentFrame == MAX_FRAMES_IN_FLIGHT - 1 ? 0 : currentFrame + 1;

            glfwPollEvents();
        }

        device.waitIdle();
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

}
