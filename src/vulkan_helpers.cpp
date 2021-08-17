#include "mycad/vulkan_helpers.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "glm/gtc/matrix_transform.hpp"

#include "shaders/vert.h"
#include "shaders/frag.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>

uint32_t findValidMemoryType(ChosenPhysicalDevice const & cpd, uint32_t allowedMask, vk::MemoryPropertyFlags reqs);
void createBuffer(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd,
                  vk::DeviceSize size, vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags props,
                  uptrBuffer & buffer, uptrMemory & memory);
void createImage(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd,
                 uint32_t width, uint32_t height,
                 vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                 vk::MemoryPropertyFlags props,
                 uptrImage & image,
                 uptrMemory & memory);

vk::VertexInputBindingDescription VertexBindingDescription{
    .binding = 0,
    .stride  = sizeof(Vertex),
    .inputRate = vk::VertexInputRate::eVertex
};

std::array<vk::VertexInputAttributeDescription, 3> VertexAttributeDescriptions{{
    {
        .location = 0,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(Vertex, pos)
    },
    {
        .location = 1,
        .binding = 0,
        .format = vk::Format::eR32G32B32Sfloat,
        .offset = offsetof(Vertex, color)
    },
    {
        .location = 2,
        .binding = 0,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(Vertex, texCoord)
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
    std::cout << '\n';
}

ApplicationData::~ApplicationData()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::makeInstance()
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

    instance = std::make_unique<vk::raii::Instance>(context, instanceCreateInfo);

    // add newline after validation performance warning
    std::cout << '\n';
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
    auto [res, imgIndex] = pld->scd->swapchain->acquireNextImage(std::numeric_limits<uint64_t>::max(), *imageAvailableSems.at(currentFrame), nullptr);
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
    void* data = pld->uniformMemories.at(imgIndex).mapMemory(0, sizeof(mvpMatrix));
    memcpy(data, &mvpMatrix, sizeof(mvpMatrix));
    pld->uniformMemories.at(imgIndex).unmapMemory();

    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    // submit the command to the graphics queue
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*imageAvailableSems.at(currentFrame)),
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &(*pld->commandBuffers->at(imgIndex)),
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &(*renderFinishedSems.at(currentFrame))
    };

    device->resetFences({frameFence});

    pld->graphicsQueue->submit({submitInfo}, frameFence);

    // submit something or other to the "present" queue (this draws!)
    vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &(*renderFinishedSems.at(currentFrame)),
        .swapchainCount = 1,
        .pSwapchains = &(**pld->scd->swapchain),
        .pImageIndices = &imgIndex
    };

    vk::Result presres;
    try
    {
        presres = pld->presentQueue->presentKHR(presentInfo);
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
    makeInstance();
    cpd = std::make_unique<ChosenPhysicalDevice>(*instance, window);
    makeLogicalDevice();

    // TODO: find a way to avoid duplicating this in rebuildPipeline
    pld = std::make_unique<PipelineData>(window, *cpd, *device);

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

    imagesInFlight = std::vector<std::optional<std::size_t>>(pld->scd->views.size(), std::nullopt);

    mvpMatrix.view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                  glm::vec3(0.0f, 0.0f, 0.0f),
                                  glm::vec3(0.0f, 0.0f, 1.0f));
    mvpMatrix.proj = glm::perspective(glm::radians(45.0f),
                                      pld->scd->extent.width / (float) pld->scd->extent.height,
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
    std::cout << "Looking for an appropriate physical device:\n";
    for (; whichDevice < devices.size(); whichDevice++)
    {
        // Check for required device extensions
        auto &device = devices.at(whichDevice);

        std::cout << "    Checking device " << device.getProperties().deviceName << '\n';

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for(const auto& deviceExtension : device.enumerateDeviceExtensionProperties())
        {
            requiredExtensions.erase(deviceExtension.extensionName);
        }

        if(not requiredExtensions.empty())
        {
            std::cout << "    Could not find all required extensions, trying again...\n";
            continue;
        }

        // Ensure appropriate swap chain support
        auto surfaceFormats      = device.getSurfaceFormatsKHR(**surface);
        auto surfacePresentModes = device.getSurfacePresentModesKHR(**surface);
        if (surfaceFormats.empty() || surfacePresentModes.empty())
        {
            std::cout << "    Could not find appropriate swapchain support, trying again...\n";
            continue;
        }

        // Check for anisotropy support. TODO: make this optional or toggleable
        if(not device.getFeatures().samplerAnisotropy)
        {
            std::cout << "    Could not find anisotropy support, trying again...\n";
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

    if(not foundGraphicsQueue)
    {
        std::cerr << "    Unable to find a Device with graphics support" << std::endl;
        std::exit(1);
    }

    if(not foundSurfaceQueue)
    {
        std::cerr << "    Unable to find a Device with support for the appropriate surface queue" << std::endl;
        std::exit(1);
    }

    if(not foundTransferQueue)
    {
        std::cout << "    Could not find a distinct transfer queue. Falling back to graphic queue." << std::endl;
        transferFamilyQueueIndex = graphicsFamilyQueueIndex;
    }

    std::cout << "        Device Chosen!\n";
    physicalDevice = std::make_unique<vk::raii::PhysicalDevice>(instance, *devices.at(whichDevice));
}

void Renderer::makeLogicalDevice()
{
    // Set up the logical device
    vk::PhysicalDeviceFeatures deviceFeatures{
        .samplerAnisotropy = true
    };

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for(uint32_t queueIndex : cpd->queueIndices)
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

    device = std::make_unique<vk::raii::Device>(*cpd->physicalDevice, deviceInfo);
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

PipelineData::PipelineData(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device)
{
    graphicsQueue = std::make_unique<vk::raii::Queue>(device, cpd.graphicsFamilyQueueIndex, 0);
    presentQueue = std::make_unique<vk::raii::Queue>(device, cpd.presentFamilyQueueIndex, 0);
    transferQueue = std::make_unique<vk::raii::Queue>(device, cpd.transferFamilyQueueIndex, 0);

    rebuild(window, cpd, device);
}

void PipelineData::rebuild(GLFWwindow * window, ChosenPhysicalDevice const & cpd, vk::raii::Device const & device)
{
    // first, reset the data that we're rebuild
    scd                 = nullptr;
    commandBuffers      = nullptr;
    commandPool         = nullptr;
    transferCommandPool = nullptr;
    descriptorSets      = nullptr;
    textureImage        = nullptr;
    textureImageView    = nullptr;
    textureImageMemory  = nullptr;
    textureSampler      = nullptr;
    framebuffers.clear();

    // then rebuild
    scd = std::make_unique<SwapchainData>(window, cpd, device);

    makeCommands(device, cpd);

    vk::DeviceSize uniformSize = sizeof(MVPBufferObject);
    for (std::size_t i = 0; i < scd->views.size(); i++)
    {
        // tmps
        uptrBuffer buf;
        uptrMemory mem;

        createBuffer(device, cpd, uniformSize, vk::BufferUsageFlagBits::eUniformBuffer,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     buf, mem);
        uniformBuffers.emplace_back(device, **buf.release());
        uniformMemories.emplace_back(device, **mem.release());
    }

    setupTextures(device, cpd);

    // TODO: find a better placo for this
    setupDescriptors(device);

    makeRenderPass(device);
    makePipeline(device);

    makeFramebuffers(device);
}

void PipelineData::makeCommands(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd)
{
    vk::CommandPoolCreateInfo poolInfo{
        .queueFamilyIndex = cpd.graphicsFamilyQueueIndex
    };

    commandPool = std::make_unique<vk::raii::CommandPool>(device, poolInfo);
    // Create the command buffers
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = static_cast<uint32_t>(scd->views.size())
    };

    poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    poolInfo.queueFamilyIndex = cpd.transferFamilyQueueIndex;
    transferCommandPool = std::make_unique<vk::raii::CommandPool>(device, poolInfo);

    commandBuffers = std::make_unique<vk::raii::CommandBuffers>(device, allocateInfo);
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

    if (pld)
    {
        pld->rebuild(window, *cpd, *device);
    }
    else
    {
        pld = std::make_unique<PipelineData>(window, *cpd, *device);
    }

    // TODO: move this, or find a better name for this method
    setupBuffers();
    recordDrawCommands();
}

void PipelineData::makeFramebuffers(vk::raii::Device const & device)
{
    // create framebuffers
    for(const auto& swapchainImageView : scd->views)
    {
        vk::FramebufferCreateInfo createInfo{
            .renderPass = **renderPass,
            .attachmentCount = 1,
            .pAttachments = &(*swapchainImageView),
            .width = scd->extent.width,
            .height = scd->extent.height,
            .layers = 1
        };

        framebuffers.emplace_back(device, createInfo);
    }

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
    for(std::size_t i = 0; i < pld->commandBuffers->size(); i++)
    {
        vk::CommandBufferBeginInfo beginInfo{};

        const auto& commandBuffer = pld->commandBuffers->at(i);

        //==== begin command
        commandBuffer.begin(beginInfo);

        vk::ClearValue clearColor = {std::array<float, 4>{0.2f, 0.3f, 0.3f, 1.0f}};

        vk::RenderPassBeginInfo renderPassBeginInfo{
            .renderPass = **pld->renderPass,
            .framebuffer = *pld->framebuffers.at(i),
            .renderArea = {
                .offset = {0, 0},
                .extent = pld->scd->extent
            },
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        //======== begin render pass
        commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, **pld->pipeline);
        commandBuffer.bindVertexBuffers(0, {**vertexBuffer}, {0});
        commandBuffer.bindIndexBuffer(**indexBuffer, 0, vk::IndexType::eUint16);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, **pld->pipelineLayout, 0, *pld->descriptorSets->at(i), {});
        commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        commandBuffer.endRenderPass();
        //======== end render pass

        commandBuffer.end();
        //==== end command
    }
}

void PipelineData::makeRenderPass(vk::raii::Device const & device)
{
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

    renderPass = std::make_unique<vk::raii::RenderPass>(device, renderPassInfo);
}

void PipelineData::makePipeline(vk::raii::Device const & device)
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

    pipelineLayout = std::make_unique<vk::raii::PipelineLayout>(device, pipelineLayoutInfo);

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

    pipeline =  std::make_unique<vk::raii::Pipeline>(device, nullptr, pipelineInfo);
}

uint32_t findValidMemoryType(ChosenPhysicalDevice const & cpd, uint32_t allowedMask, vk::MemoryPropertyFlags reqs)
{
    vk::PhysicalDeviceMemoryProperties availMem = cpd.physicalDevice->getMemoryProperties();

    // Pick a memory type to use for the buffer
    for(uint32_t i = 0; i < availMem.memoryTypeCount; i++)
    {
        if (allowedMask & (1 << i) &&
            (availMem.memoryTypes[i].propertyFlags & reqs) == reqs)
        {
            return i;
        }
    }

    std::cerr << "Unable to find suitable memory type. Bailing out\n";
    std::exit(1);
}

void createBuffer(vk::raii::Device const & device,
                  ChosenPhysicalDevice const & cpd,
                  vk::DeviceSize size,
                  vk::BufferUsageFlags usage,
                  vk::MemoryPropertyFlags props,
                  uptrBuffer & buffer,
                  uptrMemory & memory)
{
    vk::BufferCreateInfo bufferInfo {
        .size = size,
        .usage = usage,
        .sharingMode = vk::SharingMode::eConcurrent,
        .queueFamilyIndexCount = static_cast<uint32_t>(cpd.queueIndices.size()),
        .pQueueFamilyIndices = cpd.queueIndices.data()
    };
    buffer = std::make_unique<vk::raii::Buffer>(device, bufferInfo);

    vk::MemoryRequirements memReqs = buffer->getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findValidMemoryType(cpd, memReqs.memoryTypeBits, props)
    };

    memory = std::make_unique<vk::raii::DeviceMemory>(device, allocInfo);
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

    createBuffer(*device, *cpd, verticesSize + indicesSize,
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
    createBuffer(*device, *cpd, verticesSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 vertexBuffer, vertexBufferMemory);
    createBuffer(*device, *cpd, indicesSize,
                 vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                 vk::MemoryPropertyFlagBits::eDeviceLocal,
                 indexBuffer, indexBufferMemory);

    // Move the data from the staging area to the vertex buffer
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **pld->transferCommandPool,
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

    pld->transferQueue->submit({submitInfo});
    pld->transferQueue->waitIdle();
}

void PipelineData::setupDescriptors(vk::raii::Device const & device)
{
    // Setup the DescriptorSetLayouts
    vk::DescriptorSetLayoutBinding uniformBinding{
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
    };

    vk::DescriptorSetLayoutBinding samplerBinding{
        .binding = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eFragment
    };

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings{uniformBinding, samplerBinding};
    vk::DescriptorSetLayoutCreateInfo createInfo{
        .bindingCount = bindings.size(),
        .pBindings = bindings.data()
    };

    descriptorLayout = std::make_unique<vk::raii::DescriptorSetLayout>(device, createInfo);

    // Next, create a DescriptorPool
    std::array<vk::DescriptorPoolSize, 2> poolSizes
    {{
        {
            .type            = vk::DescriptorType::eUniformBuffer,
            .descriptorCount = static_cast<uint32_t>(scd->images.size())
        },
        {
            .type            = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = static_cast<uint32_t>(scd->images.size())
        }
    }};

    // TODO enable vulkan best practice validation to catch errors like
    // too-small pool size
    vk::DescriptorPoolCreateInfo poolInfo{
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = static_cast<uint32_t>(scd->images.size()),
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    descriptorPool = std::make_unique<vk::raii::DescriptorPool>(device, poolInfo);

    // Allocate the descriptor sets themselves
    std::vector<vk::DescriptorSetLayout> layouts(scd->images.size(), **descriptorLayout);

    vk::DescriptorSetAllocateInfo allocInfo{
        .descriptorPool = **descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    descriptorSets = std::make_unique<vk::raii::DescriptorSets>(device, allocInfo);

    for (std::size_t i = 0; i < layouts.size(); i++)
    {
        vk::DescriptorBufferInfo bufferInfo{
            .buffer = *uniformBuffers.at(i),
            .offset = 0,
            .range = sizeof(MVPBufferObject)
        };

        vk::DescriptorImageInfo imageInfo{
            .sampler = **textureSampler,
            .imageView = **textureImageView,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{{
            {
                .dstSet = *descriptorSets->at(i),
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pBufferInfo = &bufferInfo
            },
            {
                .dstSet = *descriptorSets->at(i),
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &imageInfo
            }
        }};

        device.updateDescriptorSets(descriptorWrites, {});
    }

}

void createImage(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd,
                 uint32_t width, uint32_t height,
                 vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                 vk::MemoryPropertyFlags props,
                 uptrImage & image,
                 uptrMemory & memory)
{
    vk::ImageCreateInfo imgInfo{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent{
            .width = width,
            .height = height,
            .depth = 1
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined,
    };

    image = std::make_unique<vk::raii::Image>(device, imgInfo);

    // allocate memory for the image
    vk::MemoryRequirements memReqs = image->getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = findValidMemoryType(cpd, memReqs.memoryTypeBits, props)
    };

    memory = std::make_unique<vk::raii::DeviceMemory>(device, allocInfo);
    image->bindMemory(**memory, 0);
}

void PipelineData::setupTextures(vk::raii::Device const & device, ChosenPhysicalDevice const & cpd)
{
    // load the file
    int width, height, channels;
    const char * fpath = "../textures/Beer_Flight_350_350.jpg";
    stbi_uc* pixels = stbi_load(fpath, &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels)
    {
        std::cerr << "\n\nCould not load: " << fpath << '\n';
        std::cerr << "Note: this development build of mycad-vk assumes the following:\n\n";
        std::cerr << "    1. out-of-source cmake build\n"
                     "    2. executable run from build dir, i.e. `./bin/mycad-vk`\n"
                     "    3. the project directory contains a sub-directory named textures\n"
                     "    4. there is a file name two-types-of-beer-1978012_512_512.jpg in that direcory\n\n"
                     "This will all be made better, of course, in an actual release.\n";
        std::exit(1);
    }

    vk::DeviceSize imgSize = width * height * 4;

    // create a staging buffer
    uptrBuffer stagingBuffer;
    uptrMemory stagingBufferMemory;

    createBuffer(device, cpd, imgSize,
                 vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                 stagingBuffer, stagingBufferMemory);

    void* data = stagingBufferMemory->mapMemory(0, imgSize);
    memcpy(data, pixels, (std::size_t) imgSize);
    stagingBufferMemory->unmapMemory();

    stbi_image_free(pixels);

    // make the vulkan vk::raii::Image for the texture
    createImage(device, cpd, width, height,
                vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

    // transfer the data from staging to buffer
    //     1) transition image from staging buffer to the texture
    transitionImageLayout(
        device,
        *textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal
    );
    //     2) actually copy the data
    //     TODO: abstract these three lines a bit (incl. CommandBuffer(s) below)
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **transferCommandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    vk::raii::CommandBuffers cbufs(device, allocateInfo);
    vk::raii::CommandBuffer const & buf = cbufs.at(0);

    vk::BufferImageCopy region{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {
            .width = static_cast<uint32_t>(width), 
            .height = static_cast<uint32_t>(height),
            .depth = 1
        }
    };

    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };
    buf.begin(beginInfo);
    buf.copyBufferToImage(
        **stagingBuffer,
        **textureImage,
        vk::ImageLayout::eTransferDstOptimal,
        {region}
    );
    buf.end();

    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &(*buf),
    };

    transferQueue->submit({submitInfo});
    transferQueue->waitIdle();

    //     3) transition from the texture image to the shader
    transitionImageLayout(
        device,
        *textureImage,
        vk::Format::eR8G8B8A8Srgb,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal
    );
    // end transfer the data from staging to buffer

    // Create an ImageView for the texture
    vk::ImageViewCreateInfo imageViewCreateInfo{
        .image = **textureImage,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Srgb,
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
    textureImageView = std::make_unique<vk::raii::ImageView>(device, imageViewCreateInfo);

    // create a sampler for the texture
    vk::SamplerCreateInfo samplerInfo{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        .anisotropyEnable = true,
        .maxAnisotropy = cpd.physicalDevice->getProperties().limits.maxSamplerAnisotropy,
        .compareEnable = true,
        .compareOp = vk::CompareOp::eAlways,
        .minLod = 0.0f,
        .maxLod = 0.0f,
        .borderColor = vk::BorderColor::eIntOpaqueBlack,
        .unnormalizedCoordinates = false

    };

    textureSampler = std::make_unique<vk::raii::Sampler>(device, samplerInfo);
}

void PipelineData::transitionImageLayout(vk::raii::Device const & device, vk::raii::Image const & img, vk::Format /*fmt*/, vk::ImageLayout oldLayout, vk::ImageLayout  newLayout)
{
    vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = **commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    vk::raii::CommandBuffers cbufs(device, allocateInfo);
    vk::raii::CommandBuffer const & buf = cbufs.at(0);

    vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    vk::PipelineStageFlags srcStage;
    vk::PipelineStageFlags dstStage;

    vk::ImageMemoryBarrier barrier{
        .srcAccessMask = vk::AccessFlagBits::eNoneKHR, // set below
        .dstAccessMask = vk::AccessFlagBits::eNoneKHR, // set below
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = *img,
        .subresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };


    if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(oldLayout == vk::ImageLayout::eTransferDstOptimal &&
            newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStage = vk::PipelineStageFlagBits::eTransfer;
        dstStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        std::cerr << "I don't know how to support that layout transition!\n";
        std::exit(1);
    }

    buf.begin(beginInfo);

    buf.pipelineBarrier(
        srcStage, dstStage,
        vk::DependencyFlagBits::eByRegion,
        {}, {}, {barrier}
    );

    buf.end();

    vk::SubmitInfo submitInfo{
        .commandBufferCount = 1,
        .pCommandBuffers = &(*buf),
    };

    graphicsQueue->submit({submitInfo});
    graphicsQueue->waitIdle();
}
