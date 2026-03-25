#pragma once

#include <vulkan/vulkan.h>

#include <vector>


namespace rk {
    // fwd
    class VulkanApp;
    class SwapChain;
    
    class PhysicalDevice {
    public:
        void create(const VulkanApp* app);
        bool checkExtensionSupport(VkPhysicalDevice device, const VulkanApp* app) const;


        VkPhysicalDevice get() const { return m_physicalDevice; }
        const std::vector<VkQueueFamilyProperties>* getQueueFamiliesCache() const { return &m_queues; }

    private:
        bool isSuitable(VkPhysicalDevice device, const VulkanApp* app) const;
        void saveQueuesCache();
        std::vector<VkQueueFamilyProperties> getQueuesFromDevice(VkPhysicalDevice physicalDevice) const;

        VkPhysicalDevice m_physicalDevice = nullptr;
        std::vector<VkQueueFamilyProperties> m_queues;
    };
}

