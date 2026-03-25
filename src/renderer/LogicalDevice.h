#pragma once

#include "VulkanFwd.h"


namespace rk {
    // fwd
    class PhysicalDevice;
    class VulkanApp;

    class LogicalDevice {
    public:
        void create(const VulkanApp* app);

        VkDevice get() const { return m_logicalDevice; }
        VkQueue getGraphicsQueue() const { return m_graphicsQueue; }
        VkQueue getPresentQueue() const { return m_presentQueue; }
        VkQueue getTransferQueue() const { return m_transferQueue; }

    private:
        VkDevice m_logicalDevice = nullptr;

        // queues
        VkQueue m_graphicsQueue = nullptr;
        VkQueue m_presentQueue = nullptr;
        VkQueue m_transferQueue = nullptr;
    };
}
