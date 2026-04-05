#include "Texture.h"

#include <cassert>

#include <stb_image.h>

#include <vulkan/vulkan.h>

#include "core/Utils.h"
#include "core/VulkanApp.h"
#include "VulkanEnums.h"
#include "resources/TextureAtlas.h"


void rk::Texture::create(const char* texturePath, SamplerFilter filter, SamplerMode mode) {
    int width, height;
    auto imageData = stbi_load(texturePath, &width, &height, nullptr, 4);

    if (!imageData)
        assert(false && "failed to load texture image");

    create(imageData, width, height, filter, mode);

    // free data in ram
    stbi_image_free(imageData);
}

void rk::Texture::create(const resources::TextureAtlas& textureAtlas, SamplerFilter filter, SamplerMode mode) {
    create(textureAtlas.data(), textureAtlas.width(), textureAtlas.height(), filter, mode);
}

void rk::Texture::create(const void* data, i32  width, i32 height, SamplerFilter filter, SamplerMode mode) {
    u64 imageSize = width * height * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    utl::createBuffer(imageSize, stagingBuffer, stagingBufferMemory, BufferUsage::TRANSFER_SRC,
        MemoryType::HOST_VISIBLE | MemoryType::HOST_COHERENT);

    utl::copyDataToStagingBuffer(imageSize, stagingBufferMemory, data);



    utl::createImage(width, height, m_image, m_imageMemory, Formats::RGBA8_SRGB, MemoryType::DEVICE_LOCAL,
        ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED);


    utl::transitionImageLayout(m_image, Formats::RGBA8_SRGB, ImageLayout::UNDEFINED, ImageLayout::TRANSFER_DST_OPTIMAL);
    utl::copyBufferToImage(width, height, stagingBuffer, m_image);
    utl::transitionImageLayout(m_image, Formats::RGBA8_SRGB, ImageLayout::TRANSFER_DST_OPTIMAL, ImageLayout::SHADER_READ_ONLY_OPTIMAL);

    m_imageView = utl::createImageView(m_image, Formats::RGBA8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    createSampler(filter, mode);

    // clear buffer and memory staging
    vkDestroyBuffer(vulkanApp::getLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(vulkanApp::getLogicalDevice(), stagingBufferMemory, nullptr);
}

void rk::Texture::destroy() const {
    auto logicalDevice = vulkanApp::getLogicalDevice();

    vkDestroySampler(logicalDevice, m_sampler, nullptr);
    vkDestroyImageView(logicalDevice, m_imageView, nullptr);
    vkDestroyImage(logicalDevice, m_image, nullptr);
    vkFreeMemory(logicalDevice, m_imageMemory, nullptr);
}

void rk::Texture::createSampler(SamplerFilter filter, SamplerMode mode) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = (VkFilter)filter;
    samplerInfo.minFilter = (VkFilter)filter;
    samplerInfo.addressModeU = (VkSamplerAddressMode)mode;
    samplerInfo.addressModeV = (VkSamplerAddressMode)mode;
    samplerInfo.addressModeW = (VkSamplerAddressMode)mode;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(vulkanApp::getLogicalDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
        assert(false && "failed to create texture sampler!");
}
