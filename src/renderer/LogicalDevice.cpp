#include "LogicalDevice.h"

#include "VulkanApp.h"
#include "defs.h"

#include <cassert>
#include <set>
#include <vector>

#include "PhysicalDevice.h"
#include "Utils.h"


void rk::LogicalDevice::create(const VulkanApp* app) {
    // grab queue family indices
    auto index = app->findQueueFamilies(app->physicalDevice.get());

    // create device queue create info
    const f32 queuePriority = 1.0f;

    // we need to create a queue for each unique queue family (we use a std::set why graphics and present can have the same index)
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = { index.graphicsFamily.value(), index.presentFamily.value(), index.transferFamily.value() };
    for (u32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // create logical device create info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledLayerCount = 0;
    createInfo.enabledExtensionCount = std::size(utils::deviceExtensions);
    createInfo.ppEnabledExtensionNames = utils::deviceExtensions;

    // create the logical device
    if (vkCreateDevice(app->physicalDevice.get(), &createInfo, nullptr, &m_logicalDevice) != VK_SUCCESS)
        assert(false && "failed to create logical device!");

    // get graphics and present queues
    vkGetDeviceQueue(m_logicalDevice, index.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_logicalDevice, index.presentFamily.value(), 0, &m_presentQueue);
    vkGetDeviceQueue(m_logicalDevice, index.transferFamily.value(), 0, &m_transferQueue);
}


