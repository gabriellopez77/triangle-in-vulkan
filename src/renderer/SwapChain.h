#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "defs.h"


struct GLFWwindow;

namespace rk {
    // fwd
    class VulkanApp;
    class RenderPass;
    class LogicalDevice;

    class SwapChain {
    public:
        struct SupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        void create(const VulkanApp* app);
        void clear(const VulkanApp* app);
        void recreate(const VulkanApp* app);

        u32 getWidth() const { return m_screenSize.width; }
        u32 getHeight() const { return m_screenSize.height; }
        VkExtent2D getSize() const { return m_screenSize; }

        u32 getImagesCount() const { return m_imagesCount; }
        u32 getImageIndex() const { return m_currentImageIndex; }

        VkFramebuffer getFramebuffer(u32 index) const { return m_framebuffers[index]; }
        VkFormat getImageFormat() const { return m_swapChainImageFormat; }
        VkSurfaceKHR getSurface() const { return m_surface; }
        VkSwapchainKHR& getSwapChain() { return m_swapChain; }

        u32 getOneImage(VkDevice device, VkSemaphore semaphore);

        void tryRecreate(const VulkanApp* app, VkResult, const VkFence* fence);
        void needRecreate() { m_resized = true; }
        void createSurface(const VulkanApp* app, GLFWwindow* window);
        void createImageViews(const VulkanApp* app);
        void createFramebuffers(const VulkanApp* app);
        SupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    private:
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);

        u32 m_currentImageIndex = 0;

        bool m_resized = false;

        u32 m_imagesCount = 0;

        VkExtent2D m_screenSize = {};
        VkSurfaceKHR m_surface = nullptr;
        VkSwapchainKHR m_swapChain = nullptr;
        VkFormat m_swapChainImageFormat = {};

        std::vector<VkImage> m_images;
        std::vector<VkImageView> m_imageViews;
        std::vector<VkFramebuffer> m_framebuffers;
    };
}