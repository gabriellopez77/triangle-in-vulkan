#include "PhysicalDevice.h"

#include <cassert>
#include <vector>
#include <cstring>
#include "defs.h"

#include "VulkanApp.h"
#include "SwapChain.h"
#include "Utils.h"


void rk::PhysicalDevice::create(const VulkanApp* app) {
    // get physical devices count
    u32 deviceCount = 0;
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        assert(false && "Failed to find GPUs with Vulkan support!");
    }

    // get physical devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(app->instance, &deviceCount, devices.data());

    // check each device for suitability and choice the first one that is suitable
    for (int i = 0; i < deviceCount; i++) {
        if (isSuitable(devices[i], app)) {
            m_physicalDevice = devices[i];
            break;
        }
    }

    if (m_physicalDevice == nullptr) {
        assert(false && "Failed to find a suitable GPU!");
    }

    // get queues families and saves it in cache
    saveQueuesCache();
}

bool rk::PhysicalDevice::checkExtensionSupport(VkPhysicalDevice device, const VulkanApp* app) const {
    // get device extension count
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    // get device extensions
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    for (auto extensionName : utils::deviceExtensions) {
        bool supported = false;

        for (auto extensionProperties : availableExtensions)
            if (strcmp(extensionName, extensionProperties.extensionName))
                supported = true;

        if (!supported)
            return false;
    }

    return true;
}

bool rk::PhysicalDevice::isSuitable(VkPhysicalDevice device, const VulkanApp* app) const {
    // get queues families supported by the device
    auto deviceQueuesFamilies = getQueuesFromDevice(device);

    // check if the device has a graphics queue family
    auto indices = app->findQueueFamilies(device, &deviceQueuesFamilies);

    // check if device supports all required extensions
    bool extensionsSupported = checkExtensionSupport(device, app);

    bool swapChainAdequate = false;

    // if extensions are supported, check if the swap chain is adequate
    if (extensionsSupported) {
        auto swapChainSupport = app->swapChain.querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void rk::PhysicalDevice::saveQueuesCache() {
    // get queue family properties count
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);

    // get queue family properties
    m_queues.resize(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queues.data());
}

std::vector<VkQueueFamilyProperties> rk::PhysicalDevice::getQueuesFromDevice(VkPhysicalDevice physicalDevice) const {
    // get queue family properties count
    u32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    // get queue family properties
    std::vector<VkQueueFamilyProperties> queues(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queues.data());

    return queues;
}
