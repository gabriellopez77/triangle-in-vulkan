#include "VertexBuffer.h"

#include <cassert>
#include <cstring>
#include <bits/ios_base.h>

#include <vulkan/vulkan.h>

#include "VulkanApp.h"


namespace rk {
    void VertexBuffer::createVertexBuffer(const VulkanApp* app, u64 size, const void* data) {
        createSendBuffer(app, size, data, &m_buffer, &m_memory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    void VertexBuffer::createIndexBuffer(const VulkanApp *app, u64 size, const void *data, i32 indexType) {
        createSendBuffer(app, size, data, &m_indexBuffer, &m_indexMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

        m_indexBufferType = indexType;
    }

    void VertexBuffer::destroy(VkDevice device) const {
        vkDestroyBuffer(device, m_buffer, nullptr);
        vkFreeMemory(device, m_memory, nullptr);

        vkDestroyBuffer(device, m_indexBuffer, nullptr);
        vkFreeMemory(device, m_indexMemory, nullptr);
    }

    void VertexBuffer::bind(VkCommandBuffer command) const {
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(command, 0, 1, &m_buffer, offsets);

        if (m_indexBuffer != nullptr)
            vkCmdBindIndexBuffer(command, m_indexBuffer, 0, (VkIndexType)m_indexBufferType);
    }

    u32 VertexBuffer::findMemoryType(const VulkanApp* app, u32 type, u32 properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(app->physicalDevice.get(), &memProperties);

        for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if (type & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        assert(false && "failed to find suitable memory type!");
    }

    void VertexBuffer::createBuffer(const VulkanApp* app, u64 size, VkBuffer* buffer, VkDeviceMemory* memory, u32 usage, u32 properties) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // create buffer
        if (vkCreateBuffer(app->logicalDevice.get(), &bufferInfo, nullptr, buffer) != VK_SUCCESS)
            assert(false && "failed to create buffer!");

        // get memory requirements
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(app->logicalDevice.get(), *buffer, &requirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(app, requirements.memoryTypeBits, properties);

        // allocate buffer memory
        if (vkAllocateMemory(app->logicalDevice.get(), &allocInfo, nullptr, memory) != VK_SUCCESS)
            assert(false && "failed to allocate buffer memory!");

        // bind memory to buffer
        vkBindBufferMemory(app->logicalDevice.get(), *buffer, *memory, 0);
    }

    void VertexBuffer::copyBuffer(const VulkanApp* app, VkBuffer src, VkBuffer dst, u64 size) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = app->getCommandPool();

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(app->logicalDevice.get(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(app->logicalDevice.getTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(app->logicalDevice.getTransferQueue());

        vkFreeCommandBuffers(app->logicalDevice.get(), app->getCommandPool(), 1, &commandBuffer);
    }

    void VertexBuffer::createSendBuffer(const VulkanApp* app, u64 size, const void* data, VkBuffer* buffer, VkDeviceMemory* memory, u32 type) {
        // create staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(app, size, &stagingBuffer, &stagingBufferMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // upload data
        void* data2;
        vkMapMemory(app->logicalDevice.get(), stagingBufferMemory, 0, size, 0, &data2);
        std::memcpy(data2, data, size);
        vkUnmapMemory(app->logicalDevice.get(), stagingBufferMemory);

        // create buffer in vram
        createBuffer(app, size, buffer, memory,VK_BUFFER_USAGE_TRANSFER_DST_BIT | type,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyBuffer(app, stagingBuffer, *buffer, size);

        vkDestroyBuffer(app->logicalDevice.get(), stagingBuffer, nullptr);
        vkFreeMemory(app->logicalDevice.get(), stagingBufferMemory, nullptr);
    }
}
