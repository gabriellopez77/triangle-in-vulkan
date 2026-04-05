#pragma once

#include "Utils.h"


namespace rk {
    // fwd
    enum class Formats : i32;

    class SwapChain {
    public:


        static bool isAdequate(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR surface);

        void create();
        void clear() const;
        void createFramebuffers();
        void recreate(VkFence fence, i32 width, i32 height);

        u32 getWidth() const { return m_screenSize.width; }
        u32 getHeight() const { return m_screenSize.height; }
        Extent2D getSize() const { return m_screenSize; }

        u32 getImageIndex() const { return m_currentImageIndex; }

        VkFramebuffer getFramebuffer(u32 index) const { return m_framebuffers[index]; }
        Formats getImageFormat() const { return m_imagesFormat; }
        VkSurfaceKHR getSurface() const { return m_surface; }
        VkSwapchainKHR& get() { return m_swapChain; }

        u32 getOneImage(VkDevice device, VkSemaphore semaphore);

        void setSurface(VkSurfaceKHR surface) { m_surface = surface; };


    private:
        void createImageViews();

        u32 m_currentImageIndex = 0;

        Extent2D m_screenSize = {};
        VkSurfaceKHR m_surface = nullptr;
        VkSwapchainKHR m_swapChain = nullptr;
        Formats m_imagesFormat = {};

        // depth buffer
        VkImage m_depthImage = nullptr;
        VkDeviceMemory m_depthImageMemory = nullptr;
        VkImageView m_depthImageView = nullptr;

        VkImage m_images[utl::FRAMES_COUNT] = {};
        VkImageView m_imageViews[utl::FRAMES_COUNT] = {};
        VkFramebuffer m_framebuffers[utl::FRAMES_COUNT] = {};
    };
}