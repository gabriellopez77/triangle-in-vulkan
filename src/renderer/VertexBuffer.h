#pragma once

#include "VulkanFwd.h"

#include "defs.h"


namespace rk {
    //fwd
    class VulkanApp;

    class VertexBuffer {
    public:
        void createVertexBuffer(const VulkanApp* app, u64 size, const void* data);
        void createIndexBuffer(const VulkanApp* app, u64 size, const void* data, i32 indexType);
        void destroy(VkDevice device) const;
        void bind(VkCommandBuffer command) const;

    private:
        void createSendBuffer(const VulkanApp* app, u64 size, const void* data, VkBuffer* buffer, VkDeviceMemory* memory, u32 type);
        void createBuffer(const VulkanApp* app, u64 size, VkBuffer* buffer, VkDeviceMemory* memory, u32 usage, u32 properties);
        void copyBuffer(const VulkanApp* app, VkBuffer src, VkBuffer dst, u64 size);
        u32 findMemoryType(const VulkanApp* app, u32 type, u32 properties);

        VkBuffer m_buffer = nullptr;
        VkDeviceMemory m_memory = nullptr;

        VkBuffer m_indexBuffer = nullptr;
        VkDeviceMemory m_indexMemory = nullptr;
        i32 m_indexBufferType = 0;
    };
}
