#pragma once

#include <vulkan/vulkan.h>

#include "defs.h"

#include <optional>
#include <vector>

struct GLFWwindow;


class VulkanApp {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    
public:
    void init(GLFWwindow* window);
    void drawFrame();
    
private:
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface(GLFWwindow* window);
    void createSwapChain(GLFWwindow* window);
    void createImageViews();
    void createGraphicsPipeline();
    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffer();
    void createSyncObjects();

    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<char> readShaderFile(const char* filePath);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);

    // extensions required by our application
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    // swap chain
    VkSwapchainKHR m_swapChain = nullptr;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkFormat m_swapChainImageFormat = {};
    VkExtent2D m_swapChainExtent = {};

    VkInstance m_instance = nullptr;
    VkPhysicalDevice m_physicalDevice = nullptr;
    VkDevice m_device = nullptr;
    VkSurfaceKHR m_surface = nullptr;
    VkPipelineLayout m_pipelineLayout = nullptr;
    VkRenderPass m_renderPass = nullptr;
    VkPipeline m_graphicsPipeline = nullptr;
    VkCommandPool m_commandPool = nullptr;
    VkCommandBuffer m_commandBuffer = nullptr;

    // queues
    VkQueue m_graphicsQueue = nullptr;
    VkQueue m_presentQueue = nullptr;

    // fence
    VkSemaphore m_imageAvailableSemaphore = nullptr;
    VkSemaphore m_renderFinishedSemaphore = nullptr;
    VkFence m_inFlightFence = nullptr;
};