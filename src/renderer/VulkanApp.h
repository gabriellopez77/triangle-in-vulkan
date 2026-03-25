#pragma once

#include <optional>
#include <vector>

#include "defs.h"
#include "PhysicalDevice.h"
#include "SwapChain.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "VertexBuffer.h"
#include "PushConstants.h"

// fwd
struct GLFWwindow;

namespace rk {
    class VulkanApp {
    public:
        static constexpr bool VALIDATION_LAYERS_ENABLED = true;

        struct QueueFamilyIndices {
            std::optional<u32> graphicsFamily;
            std::optional<u32> presentFamily;
            std::optional<u32> transferFamily;

            bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
            }
        };

        void init(GLFWwindow* window);
        void clear();
        void resize();
        void drawFrame();

        void beginFrame();
        void endFrame();

        VkCommandPool getCommandPool() const { return m_commandPool; };
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const std::vector<VkQueueFamilyProperties>* queues = nullptr) const;

        GLFWwindow* window = nullptr;
        PushConstants pushConstants;
        PhysicalDevice physicalDevice;
        SwapChain swapChain;
        RenderPass renderPass;
        GraphicsPipeline graphicsPipeline1;
        LogicalDevice logicalDevice;


        VkInstance instance = nullptr;

        VertexBuffer vertexBuffer;
    private:
        void createInstance();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();

        u32 m_currentFrame = 0;
        u32 m_currentImageIndex = 0;

        // commands
        VkCommandPool m_commandPool = nullptr;
        std::vector<VkCommandBuffer> m_commandBuffers = {};

        // fence
        std::vector<VkSemaphore> m_imageAvailableSemaphore;
        std::vector<VkSemaphore> m_renderFinishedSemaphore;
        std::vector<VkFence> m_inFlightFence;
    };
}
