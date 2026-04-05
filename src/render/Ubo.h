#pragma once

#include "core/Utils.h"


namespace rk {
    class Ubo {
    public:
        void create(u64 size);
        void destroy() const;
        void update(u64 offset, u64 size, const void* data) const;
        void updateAll(u64 offset, u64 size, const void* data) const;

        VkBuffer getBuffer(int index) const { return m_buffers[index]; }
        u64 getSize() const { return m_size; }

    private:
        u64 m_size = 0;

        VkBuffer m_buffers[utl::FRAMES_COUNT] = {};
        VkDeviceMemory m_buffersMemory[utl::FRAMES_COUNT] = {};
        void* m_buffersMapped[utl::FRAMES_COUNT] = {};
    };
}
