#include "SwapChain.h"

#include <cassert>
#include <vector>

#include <vulkan/vulkan.h>

#include "Application.h"
#include "VulkanApp.h"
#include "../VulkanEnums.h"

#include "math/Math.h"


struct SupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
SupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);


bool rk::SwapChain::isAdequate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    auto swapChainSupport = querySwapChainSupport(physicalDevice, surface);
    return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}

void rk::SwapChain::create() {
    auto logicalDevice = vulkanApp::getLogicalDevice();

    auto swapChainSupport = querySwapChainSupport(vulkanApp::getPhysicalDevice(), m_surface);

    // choose the best settings for our swap chain
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);

    assert(swapChainSupport.capabilities.minImageCount <= utl::FRAMES_COUNT && "swapChain images count not supported");


    // create swap chain create info
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_surface;
    createInfo.minImageCount = utl::FRAMES_COUNT;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = vulkanApp::findQueueFamilies();
    u32 queueFamilyIndices[] = { indices.graphics.value(), indices.present.value() };

    if (indices.graphics != indices.present) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = nullptr;

    // create the swap chain
    if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
        assert(false && "failed to create swap chain!");


    // get swap chain images
    u32 imageCount = utl::FRAMES_COUNT;
    vkGetSwapchainImagesKHR(logicalDevice, m_swapChain, &imageCount, m_images);

    m_imagesFormat = (Formats)surfaceFormat.format;
    m_screenSize = { extent.width, extent.height };

    createImageViews();

    utl::createImage(extent.width, extent.height, m_depthImage, m_depthImageMemory, Formats::DEPTH_F32,
        MemoryType::DEVICE_LOCAL, ImageUsage::DEPTH_STENCIL_ATTACHMENT);

    m_depthImageView = utl::createImageView(m_depthImage, Formats::DEPTH_F32, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void rk::SwapChain::createImageViews() {
    for (size_t i = 0; i < utl::FRAMES_COUNT; i++)
        m_imageViews[i] = utl::createImageView(m_images[i], m_imagesFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void rk::SwapChain::createFramebuffers() {
    for (int i = 0; i < utl::FRAMES_COUNT; i++) {
        VkImageView attachments[] = { m_imageViews[i], m_depthImageView };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = vulkanApp::getRenderPass();
        framebufferInfo.attachmentCount = std::size(attachments);
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_screenSize.width;
        framebufferInfo.height = m_screenSize.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vulkanApp::getLogicalDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]))
            assert(false && "failed to create framebuffer!");
    }
}

SupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SupportDetails details;

    // get physical device surface capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // get physical device surface formats count
    u32 formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // get physical device surface present modes count
    u32 presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    // get physical device surface present modes
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

u32 rk::SwapChain::getOneImage(VkDevice device, VkSemaphore semaphore) {
    auto result = vkAcquireNextImageKHR(device, m_swapChain, UINT64_MAX, semaphore, nullptr, &m_currentImageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR)
        assert(false && "failed to acquire swap chain image!");

    return m_currentImageIndex;
}

void rk::SwapChain::clear() const {
    auto logicalDevice = rk::vulkanApp::getLogicalDevice();

    for (int i = 0; i < utl::FRAMES_COUNT; i++) {
        vkDestroyFramebuffer(logicalDevice, m_framebuffers[i], nullptr);
        vkDestroyImageView(logicalDevice, m_imageViews[i], nullptr);
    }

    // depth buffer
    vkDestroyImageView(logicalDevice, m_depthImageView, nullptr);
    vkDestroyImage(logicalDevice, m_depthImage, nullptr);
    vkFreeMemory(logicalDevice, m_depthImageMemory, nullptr);

    vkDestroySwapchainKHR(logicalDevice, m_swapChain, nullptr);
}

void rk::SwapChain::recreate(VkFence fence, i32 width, i32 height) {
    // wait for command is finished
    vkWaitForFences(vulkanApp::getLogicalDevice(), 1, &fence, VK_TRUE, UINT64_MAX);

    // destroy the old swapChain
    clear();

    // create a new swapChain
    create();
    createFramebuffers();
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;
    }

    // if the preferred format isn't available, just return the first one from the list
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    // try to choose VK_PRESENT_MODE_MAILBOX_KHR mode
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    // if the preferred present mode isn't available just return VK_PRESENT_MODE_FIFO_KHR (which is guaranteed to be available)
    //return VK_PRESENT_MODE_IMMEDIATE_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;

    int width, height;
    rk::vulkanApp::application->getFrameBufferSize(&width, &height);

    VkExtent2D actualExtent = { (u32)width, (u32)height };

    // limits extent between min and max extents supported by the surface
    actualExtent.width = math::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = math::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}