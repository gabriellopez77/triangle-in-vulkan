#include "VertexBuffer.h"

#include <vulkan/vulkan.h>

#include "core/VulkanApp.h"
#include "VulkanEnums.h"


void createSendBuffer(u64 size, const void* data, VkBuffer& buffer, VkDeviceMemory& memory, rk::BufferUsage usage);

void rk::VertexBuffer::create(u64 verticesSize, const void* verticesData,
    u64 indicesSize, const u32* indicesData) {
    createSendBuffer(verticesSize, verticesData, m_buffer, m_memory, BufferUsage::VERTEX_BUFFER);
    createSendBuffer(indicesSize, indicesData, m_indexBuffer, m_indexMemory, BufferUsage::INDEX_BUFFER);
}

void rk::VertexBuffer::create(u64 verticesSize, const void* verticesData) {
    createSendBuffer(verticesSize, verticesData, m_buffer, m_memory, BufferUsage::VERTEX_BUFFER);
}

void rk::VertexBuffer::destroy(VkDevice device) const {
    vkDestroyBuffer(device, m_buffer, nullptr);
    vkFreeMemory(device, m_memory, nullptr);

    vkDestroyBuffer(device, m_indexBuffer, nullptr);
    vkFreeMemory(device, m_indexMemory, nullptr);
}

void rk::VertexBuffer::bind(VkCommandBuffer command, u32 binding) const {
    u64 offset = 0;

    vkCmdBindVertexBuffers(command, binding, 1, &m_buffer, &offset);

    if (m_indexBuffer != nullptr)
        vkCmdBindIndexBuffer(command, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void createSendBuffer(u64 size, const void* data, VkBuffer& buffer, VkDeviceMemory& memory, rk::BufferUsage usage) {
    auto logicalDevice = rk::vulkanApp::getLogicalDevice();

    // create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    rk::utl::createBuffer(size, stagingBuffer, stagingBufferMemory, rk::BufferUsage::TRANSFER_SRC,
        rk::MemoryType::HOST_VISIBLE | rk::MemoryType::HOST_COHERENT);

    // upload data
    rk::utl::copyDataToStagingBuffer(size, stagingBufferMemory, data);

    // create buffer in VRAM
    rk::utl::createBuffer(size, buffer, memory, rk::BufferUsage::TRANSFER_DST | usage, rk::MemoryType::DEVICE_LOCAL);
    rk::utl::copyBuffer(stagingBuffer, buffer, size);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}