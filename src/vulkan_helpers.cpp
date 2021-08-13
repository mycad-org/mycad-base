#include "mycad/vulkan_helpers.h"

#include "glm/gtc/matrix_transform.hpp"

#include "shaders/vert.h"
#include "shaders/frag.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

std::unique_ptr<vk::raii::Instance> makeInstance(vk::raii::Context const & context);
std::unique_ptr<vk::raii::Device> makeLogicalDevice(ChosenPhysicalDevice const & cpd);
std::vector<vk::raii::Framebuffer> makeFramebuffers(vk::raii::Device const & device, vk::raii::RenderPass const & renderPass, SwapchainData const &scd);
std::unique_ptr<ChosenPhysicalDevice> choosePhysicalDevice(
    vk::raii::Instance const & instance,
    ApplicationData const & app);

vk::VertexInputBindingDescription VertexBindingDescription{
    .binding = 0,
    .stride  = sizeof(Vertex),
    .inputRate = vk::VertexInputRate::eVertex
};

std::array<vk::VertexInputAttributeDescription, 2> VertexAttributeDescriptions{{
    {
        .location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(Vertex, pos)
    },
    {
        .location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(Vertex, color)
    }
}};

namespace {
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

        std::cerr << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }
} // namespace (anonymous)

ApplicationData::ApplicationData()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cerr << "Could not create a glfw Window" << std::endl;
        std::exit(1);
    }

    // List available extensions
    std::cout << "Available vulkan extensions: " << '\n';
    for (const auto& extension : vk::enumerateInstanceExtensionProperties())
    {
        std::cout << "    " << extension.extensionName << '\n';
    }
}

ApplicationData::~ApplicationData()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::unique_ptr<vk::raii::Instance> makeInstance(vk::raii::Context const & context)
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
    return std::make_unique<vk::raii::Instance>(context, instanceCreateInfo);
}

Renderer::~Renderer()
{
    device->waitIdle();
}

void Renderer::draw(int currentFrame)
{
    auto& frameFence      = *inFlightFences.at(currentFrame);
    // Wait for any previous frames that haven't finished yet
    [[maybe_unused]] auto waitResult = device->waitForFences({frameFence}, VK_TRUE, std::numeric_limits<uint64_t>::max());

    // Acquire an image from the swap chain
    auto [res, imgIndex] = scd->swapchain->acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSems.at(currentFrame), nullptr);
    if (res == vk::Result::eErrorOutOfDateKHR)
    {
        rebuildPipeline();
        return;
    }

    auto& maybeFenceIndex =  imagesInFlight.at(imgIndex);

    // Check if this image is still "in flight", and wait if so
    if(maybeFenceIndex.has_value())
    {
        std::size_t i = maybeFenceIndex.value();
        [[maybe_unused]] auto imageWaitRes = device->waitForFences(*inFlightFences.at(i), VK_TRUE, std::numeric_limits<uint64_t>::max());
    }

    imagesInFlight[imgIndex] = currentFrame;

    // update the uniform buffer for this imgIndex
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    mvpMatrix.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    // send latest uniform buffer data to the gpu
    void* data = uniformMemories.at(imgIndex).mapMemory(0, sizeof(mvpMatrix));
    memcpy(data, &mvpMatrix, sizeof(mvpMatrix));
    uniformMemories.at(imgIndex).unmapMemory();

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    // submit the command to the graphics queue
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*imageAvailableSems.at(currentFrame)),
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &(*commandBuffers->at(imgIndex)),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &(*renderFinishedSems.at(currentFrame))
    };

    device->resetFences({frameFence});

    graphicsQueue->submit({submitInfo}, frameFence);

    // submit something or other to the "present" queue (this draws!)
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*renderFinishedSems.at(currentFrame)),
        .swapchainCount = 1,
        .pSwapchains = &(**scd->swapchain),
        .pImageIndices = &imgIndex
    };

    vk::Result presres;
    try
    {
        presres = presentQueue->presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError & err)
    {
        framebufferResized = false;
        rebuildPipeline();
        return;
    }

    if (framebufferResized || presres == vk::Result::eSuboptimalKHR)
    {
        framebufferResized = false;
        rebuildPipeline();
    }
}

Renderer::Renderer(GLFWwindow * win, int maxFrames) : window(win)
{
    instance = makeInstance(context);
    cpd = std::make_unique<ChosenPhysicalDevice>(*instance, window);
    device = makeLogicalDevice(*cpd);
    graphicsQueue = std::make_unique<vk::raii::Queue>(*device, cpd->graphicsFamilyQueueIndex, 0);
    presentQueue = std::make_unique<vk::raii::Queue>(*device, cpd->presentFamilyQueueIndex, 0);
    transferQueue = std::make_unique<vk::raii::Queue>(*device, cpd->transferFamilyQueueIndex, 0);

    // TODO: find a way to avoid duplicating this in rebuildPipeline
    scd = std::make_unique<SwapchainData>(window, *cpd, *device);

    vk::DeviceSize uniformSize = sizeof(MVPBufferObject);
    for (std::size_t i = 0; i < scd->views.size(); i++)
    {
        // tmps
        uptrBuffer buf;
        uptrMemory mem;

        createBuffer(uniformSize, vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     buf, mem);
        uniformBuffers.emplace_back(*device, **buf.release());
        uniformMemories.emplace_back(*device, **mem.release());
    }

    rebuildPipeline();

    for(int i = 0 ; i < maxFrames; i++)
    {
        imageAvailableSems.emplace_back(*device, vk::SemaphoreCreateInfo());
        renderFinishedSems.emplace_back(*device, vk::SemaphoreCreateInfo());

        vk::FenceCreateInfo fenceInfo{
            .flags = vk::FenceCreateFlagBits::eSignaled
        };
        inFlightFences.emplace_back(*device, fenceInfo);
    }

    imagesInFlight = std::vector<std::optional<std::size_t>>(scd->views.size(), std::nullopt);

    mvpMatrix.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                  glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0f, 0.0f, 1.0f));
    mvpMatrix.proj = glm::perspective(glm::radians(45.0f),
                                      scd->extent.width / (float) scd->extent.height,
                                      0.1f, 10.0f);
    // fix for inverted Y from opengl days
    mvpMatrix.proj[1][1] *= -1;

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow * window, int, int)
        {
            auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
            renderer->framebufferResized = true;
        });
}


ChosenPhysicalDevice::ChosenPhysicalDevice(vk::raii::Instance const & instance, GLFWwindow * window)
{
    // Create a "screen surface" to render to.
    VkSurfaceKHR rawSurface;
    if (glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface) != VK_SUCCESS)
    {
        std::cerr << "Error creating a vulkan surface" << std::endl;
        std::exit(1);
    }
    // TODO: do we need to worry about this being destructed before
    // vk::raii::Instance?
    surface = std::make_unique<vk::raii::SurfaceKHR>(instance, rawSurface);

    // Set up appropriate device
    auto devices = vk::raii::PhysicalDevices(instance);
    std::size_t whichDevice = 0;
    bool foundGraphicsQueue = false;
    bool foundSurfaceQueue  = false;
    bool foundTransferQueue  = false;
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
        auto surfaceFormats      = device.getSurfaceFormatsKHR(**surface);
        auto surfacePresentModes = device.getSurfacePresentModesKHR(**surface);
        if (surfaceFormats.empty() || surfacePresentModes.empty())
        {
            continue;
        }


        // Reset in case both were not found last time
        foundGraphicsQueue  = false;
        foundSurfaceQueue   = false;
        foundTransferQueue  = false;
        auto queues = device.getQueueFamilyProperties();
        for(auto const& queue : device.getQueueFamilyProperties())
        {
            if (not foundGraphicsQueue)
            {
                if (queue.queueFlags & vk::QueueFlagBits::eGraphics)
                {
                    foundGraphicsQueue = true;
                }
                else
                {
                    graphicsFamilyQueueIndex++;
                }
            }
            if (not foundSurfaceQueue) 
            {
                if (device.getSurfaceSupportKHR(presentFamilyQueueIndex, **surface))
                {
                    foundSurfaceQueue = true;
                }
                else
                {
                    presentFamilyQueueIndex++;
                }
            }
            if (not foundTransferQueue)
            {
                if((queue.queueFlags & vk::QueueFlagBits::eTransfer) &&
                   !(queue.queueFlags & vk::QueueFlagBits::eGraphics))
                {
                    foundTransferQueue = true;
                }
                else
                {
                    transferFamilyQueueIndex++;
                }
            }

            if (foundGraphicsQueue && foundSurfaceQueue && foundTransferQueue)
            {
                queueIndices = {graphicsFamilyQueueIndex, presentFamilyQueueIndex, foundTransferQueue};
                std::ranges::sort(queueIndices);
                auto [first, last] = std::ranges::unique(queueIndices);
                queueIndices.erase(first, last);
                break;
            }
        }

        if (foundGraphicsQueue && foundSurfaceQueue && foundTransferQueue)
        {
            break;
        }
    }

    if (graphicsFamilyQueueIndex == 0 && presentFamilyQueueIndex == 0 && not foundGraphicsQueue && not foundSurfaceQueue)
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

    if(not foundTransferQueue)
    {
        std::cout << "Could not find a distinct transfer queue. Falling back to graphic queue." << std::endl;
        transferFamilyQueueIndex = graphicsFamilyQueueIndex;
    }

    physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(instance, *devices.at(whichDevice));
}

std::unique_ptr<vk::raii::Device> makeLogicalDevice(ChosenPhysicalDevice const & cpd)
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

    return std::make_unique<vk::raii::Device>(*cpd.physicalDevice, deviceInfo);
}

SwapchainData::SwapchainData(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device)
{
    // create the swap chain
    auto surfaceFormats      = cpd.physicalDevice->getSurfaceFormatsKHR(**cpd.surface);
    auto surfacePresentModes = cpd.physicalDevice->getSurfacePresentModesKHR(**cpd.surface);
    auto surfaceCapabilities = cpd.physicalDevice->getSurfaceCapabilitiesKHR(**cpd.surface);

    surfaceFormat = vk::SurfaceFormatKHR{
        .format     = vk::Format::eB8G8R8A8Srgb,
        .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear
    };

    auto presentMode = vk::PresentModeKHR::eMailbox;

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
        glfwGetFramebufferSize(window, &width, &height);

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
        .surface = **cpd.surface,
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

    swapchain = std::make_unique<vk::raii::SwapchainKHR>(device, swapchainInfo);

    // Create an image "view" for each swapchain image
    images = swapchain->getImages();
    for(const auto& swapchainImage : images)
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
        views.emplace_back(device, imageViewCreateInfo);
    }
}

void Renderer::rebuildPipeline()
{
    // Wait until window is not minimized
    int width = 0, height = 0;
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    // Let any GPU stuff finish
    device->waitIdle();

    // Clear these before re-creating them, otherwise vulkan validation layers
    // complain
    scd = nullptr;
    framebuffers.clear();
    commandBuffers = nullptr;
    commandPool = nullptr;
    transferCommandPool = nullptr;

    scd = std::make_unique<SwapchainData>(window, *cpd, *device);

    // TODO: find a better placo for this
    setupDescriptors();

    makePipelineAndRenderpass();

    framebuffers = makeFramebuffers(*device, *renderPass, *scd);

    vk::CommandPoolCreateInfo poolInfo{
        .queueFamilyIndex = cpd->graphicsFamilyQueueIndex
    };

    commandPool = std::make_unique<vk::raii::CommandPool>(*device, poolInfo);
    // Create the command buffers
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(framebuffers.size())
    };

    poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    poolInfo.queueFamilyIndex = cpd->transferFamilyQueueIndex;
    transferCommandPool = std::make_unique<vk::raii::CommandPool>(*device, poolInfo);

    commandBuffers = std::make_unique<vk::raii::CommandBuffers>(*device, allocateInfo);

    // TODO: move this, or find a better name for this method
    setupBuffers();
    recordDrawCommands();
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

/* RenderTarget makeRenderTarget(ChosenPhysicalDevice const & cpd) */
/* { */
/*     vk::raii::Device device = makeLogicalDevice(cpd); */
/*     vk::raii::Queue graphicsQueue(device, cpd.graphicsFamilyQueueIndex, 0); */
/*     vk::raii::Queue presentQueue(device, cpd.presentFamilyQueueIndex, 0); */

/*     return { */
/*         .device = std::move(device), */
/*         .graphicsQueue = std::move(graphicsQueue), */
/*         .presentQueue = std::move(presentQueue) */
/*     }; */
/* } */

void Renderer::recordDrawCommands ()
{
    // Record the commands
    for(std::size_t i = 0; i < commandBuffers->size(); i++)
    {
        vk::CommandBufferBeginInfo beginInfo{};

        const auto& commandBuffer = commandBuffers->at(i);

        //==== begin command
        commandBuffer.begin(beginInfo);

        vk::ClearValue clearColor = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};

        vk::RenderPassBeginInfo renderPassBeginInfo{
            .renderPass = *(*renderPass),
            .framebuffer = *framebuffers.at(i),
            .renderArea = {
                .offset = {0, 0},
                .extent = scd->extent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        //======== begin render pass
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pipeline);
        commandBuffer.bindVertexBuffers(0, {**vertexBuffer}, {0});
        commandBuffer.bindIndexBuffer(**indexBuffer, 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pipelineLayout, 0, *descriptorSets->at(i), {});
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        commandBuffer.endRenderPass();
        //======== end render pass

        commandBuffer.end();
        //==== end command
    }
}

void Renderer::makePipelineAndRenderpass()
{
    // Attach shaders
    vk::ShaderModuleCreateInfo vShaderInfo{
        .codeSize = vert_spv_len,
        .pCode = reinterpret_cast<const uint32_t*>(vert_spv)
    };
    vk::ShaderModuleCreateInfo fShaderInfo{
        .codeSize = frag_spv_len,
        .pCode = reinterpret_cast<const uint32_t*>(frag_spv)
    };

    vk::raii::ShaderModule vShaderModule(*device, vShaderInfo);
    vk::raii::ShaderModule fShaderModule(*device, fShaderInfo);

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
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &VertexBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(VertexAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = VertexAttributeDescriptions.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssyInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE
    };

    vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = (float) scd->extent.width,
        .height = (float) scd->extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    vk::Rect2D scissor{
        .offset = {0, 0},
        .extent = scd->extent
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
        .frontFace = vk::FrontFace::eCounterClockwise,
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

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
        .setLayoutCount = 1,
        .pSetLayouts = &(**descriptorLayout)
    };

    pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(*device, pipelineLayoutInfo);

    vk::AttachmentDescription colorAttachment{
        .format = scd->surfaceFormat.format,
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

    renderPass = std::make_unique<vk::raii::RenderPass>(*device, renderPassInfo);

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
        .layout = **pipelineLayout,
        .renderPass = **renderPass,
        .subpass = 0
    };

    pipeline =  std::make_unique<vk::raii::Pipeline>(*device, nullptr, pipelineInfo);
}

uint32_t findValidMemoryType(uint32_t allowedMask, vk::PhysicalDeviceMemoryProperties const & props, vk::MemoryPropertyFlags reqs)
{
    // Pick a memory type to use for the buffer
    for(uint32_t i = 0; i < props.memoryTypeCount; i++)
    {
        if (allowedMask & (1 << i) &&
            (props.memoryTypes[i].propertyFlags & reqs) == reqs)
        {
            return i;
        }
    }

    std::cerr << "Unable to find suitable memory type. Bailing out\n";
    std::exit(1);
}

void Renderer::createBuffer(vk::DeviceSize size,
                           vk::BufferUsageFlags usage,
                           vk::MemoryPropertyFlags props,
                           uptrBuffer & buffer,
                           uptrMemory & memory)
{
    vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eConcurrent,
        .queueFamilyIndexCount = static_cast<uint32_t>(cpd->queueIndices.size()),
        .pQueueFamilyIndices = cpd->queueIndices.data()
    };
    buffer = std::make_unique<vk::raii::Buffer>(*device, bufferInfo);

    vk::MemoryRequirements memReqs = buffer->getMemoryRequirements();
    vk::PhysicalDeviceMemoryProperties availMem = cpd->physicalDevice->getMemoryProperties();

    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findValidMemoryType(memReqs.memoryTypeBits, availMem, props)
    };

    memory = std::make_unique<vk::raii::DeviceMemory>(*device, allocInfo);
    buffer->bindMemory(**memory, 0);
}

void Renderer::setupBuffers()
{
    vk::DeviceSize verticesSize = sizeof(vertices.at(0)) * vertices.size();
    vk::DeviceSize indicesSize = sizeof(indices.at(0)) * indices.size();

    // Create the staging buffer locally - make sure it's big enough for all our
    // data
    uptrBuffer stagingBuffer = nullptr;
    uptrMemory stagingBufferMemory = nullptr;

    createBuffer(verticesSize + indicesSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    // Move vertex and index data to the staging area
    void* data = stagingBufferMemory->mapMemory(0, verticesSize);
    memcpy(data, vertices.data(), (std::size_t) verticesSize);
    stagingBufferMemory->unmapMemory();
    data = stagingBufferMemory->mapMemory(verticesSize, indicesSize);
    memcpy(data, indices.data(), (std::size_t) indicesSize);
    stagingBufferMemory->unmapMemory();

    // create the vertex and index buffers, stored as member variable
    createBuffer(verticesSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 vertexBuffer, vertexBufferMemory);
    createBuffer(indicesSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 indexBuffer, indexBufferMemory);

    // Move the data from the staging area to the vertex buffer
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **transferCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 2
    };

    vk::raii::CommandBuffers transferCommandBuffers(*device, allocateInfo);

    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    // copy vertices, buffer 0
    transferCommandBuffers.at(0).begin(beginInfo);
    vk::BufferCopy copyRegion{.size = verticesSize};
    transferCommandBuffers.at(0).copyBuffer(**stagingBuffer, **vertexBuffer, copyRegion);
    copyRegion.srcOffset = verticesSize;
    transferCommandBuffers.at(0).end();

    // copy indices, buffer 1
    transferCommandBuffers.at(1).begin(beginInfo);
    copyRegion.srcOffset = verticesSize;
    copyRegion.size = indicesSize;
    transferCommandBuffers.at(1).copyBuffer(**stagingBuffer, **indexBuffer, copyRegion);
    transferCommandBuffers.at(1).end();

    std::vector<vk::CommandBuffer> buffs;
    for (auto const & transferBuffer : transferCommandBuffers)
    {
        buffs.push_back(*transferBuffer);
    }

    vk::SubmitInfo submitInfo{
        .commandBufferCount = 2,
        .pCommandBuffers = buffs.data(),
    };

    transferQueue->submit({submitInfo});
    transferQueue->waitIdle();
}

void Renderer::setupDescriptors()
{
    // Setup the DescriptorSetLayouts
    vk::DescriptorSetLayoutBinding binding
    {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
    };

    vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = 1,
        .pBindings = &binding
    };

    descriptorLayout = std::make_unique<vk::raii::DescriptorSetLayout>(*device, createInfo);

    // Next, create a DescriptorPool
    vk::DescriptorPoolSize poolSize{
        .descriptorCount = static_cast<uint32_t>(scd->images.size())
    };

    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(scd->images.size()),
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    descriptorPool = std::make_unique<vk::raii::DescriptorPool>(*device, poolInfo);

    // Allocate the descriptor sets themselves
    std::vector<vk::DescriptorSetLayout> layouts(scd->images.size(), **descriptorLayout);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = **descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    descriptorSets = std::make_unique<vk::raii::DescriptorSets>(*device, allocInfo);

    for (std::size_t i = 0; i < layouts.size(); i++)
    {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *uniformBuffers.at(i),
            .offset = 0,
            .range = sizeof(MVPBufferObject)
        };

        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = *descriptorSets->at(i),
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo
        };

        device->updateDescriptorSets({descriptorWrite}, {});
    }

}
