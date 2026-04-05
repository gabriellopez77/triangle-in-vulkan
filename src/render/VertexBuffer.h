#pragma once

#include "core/VulkanFwd.h"

#include "Defs.h"


namespace rk {
    class VertexBuffer {
    public:
        void create(u64 verticesSize,const void* verticesData);
        void create(u64 verticesSize,
            const void* verticesData, u64 indicesSize,
            const u32* indicesData);
        void destroy(VkDevice device) const;
        void bind(VkCommandBuffer command, u32 binding) const;

    private:
        VkBuffer m_buffer = nullptr;
        VkDeviceMemory m_memory = nullptr;

        VkBuffer m_indexBuffer = nullptr;
        VkDeviceMemory m_indexMemory = nullptr;
    };
}
