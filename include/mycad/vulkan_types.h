#ifndef MYCAD_VULKAN_TYPES_HEADER
#define MYCAD_VULKAN_TYPES_HEADER

#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3native.h>

#include <memory>
#include <set>
#include <vector>

using uptrInstance          = std::unique_ptr<vk::raii::Instance>;
using uptrPhysicalDevice    = std::unique_ptr<vk::raii::PhysicalDevice>;
using uptrSurfaceKHR        = std::unique_ptr<vk::raii::SurfaceKHR>;
using VulkanIndex           = uint32_t;
using uIndices              = std::set<VulkanIndex>;
using uptrSwapchain         = std::unique_ptr<vk::raii::SwapchainKHR>;
using Images                = std::vector<VkImage>;
using uptrImageView         = std::unique_ptr<vk::raii::ImageView>;
using ImageViews            = std::vector<vk::raii::ImageView>;
using uptrDevice            = std::unique_ptr<vk::raii::Device>;
using uptrQueue             = std::unique_ptr<vk::raii::Queue>;
using uptrPipeline          = std::unique_ptr<vk::raii::Pipeline>;
using uptrPipelineLayout    = std::unique_ptr<vk::raii::PipelineLayout>;
using uptrRenderPass        = std::unique_ptr<vk::raii::RenderPass>;
using Framebuffers          = std::vector<vk::raii::Framebuffer>;
using uptrCommandPool       = std::unique_ptr<vk::raii::CommandPool>;
using uptrCommandBuffers    = std::unique_ptr<vk::raii::CommandBuffers>;
using Semaphores            = std::vector<vk::raii::Semaphore>;
using Fences                = std::vector<vk::raii::Fence>;
using MaybeIndex            = std::optional<std::size_t>;
using MaybeIndices          = std::vector<MaybeIndex>;
using uptrBuffer            = std::unique_ptr<vk::raii::Buffer>;
using uptrMemory            = std::unique_ptr<vk::raii::DeviceMemory>;
using uptrDescriptorLayout  = std::unique_ptr<vk::raii::DescriptorSetLayout>;
using Buffers               = std::vector<vk::raii::Buffer>;
using Memories              = std::vector<vk::raii::DeviceMemory>;
using uptrDescriptorPool    = std::unique_ptr<vk::raii::DescriptorPool>;
using uptrDescriptorSets    = std::unique_ptr<vk::raii::DescriptorSets>;
using uptrImage             = std::unique_ptr<vk::raii::Image>;
using uptrSampler           = std::unique_ptr<vk::raii::Sampler>;

#endif
