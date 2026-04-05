#pragma once

#include "core/VulkanFwd.h"

#include "Defs.h"


namespace resources {
    class TextureAtlas;
}

namespace rk {
    // fwd
    enum class SamplerFilter : i32;
    enum class SamplerMode : i32;

    class Texture {
    public:
        void create(const char* texturePath, SamplerFilter filter, SamplerMode mode);
        void create(const resources::TextureAtlas& textureAtlas, SamplerFilter filter, SamplerMode mode);
        void create(const void* data, i32  width, i32 height, SamplerFilter filter, SamplerMode mode);
        void destroy() const;

        VkSampler getSampler() const { return m_sampler; }
        VkImageView getImageView() const { return m_imageView; }

    private:
        void createSampler(SamplerFilter filter, SamplerMode mode);

        VkImage m_image = nullptr;
        VkDeviceMemory m_imageMemory = nullptr;
        VkImageView m_imageView = nullptr;
        VkSampler m_sampler = nullptr;
    };
}
