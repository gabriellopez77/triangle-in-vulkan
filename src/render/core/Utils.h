#pragma once

#include "Defs.h"
#include "VulkanFwd.h"

#define SHADERS_FOLDER PROJECT_ROOT_PATH"/assets/shaders/compiled"


namespace rk {
    // fwd
    enum class BufferUsage : u32;
    enum class MemoryType : u32;
    enum class Formats : i32;
    enum class ImageUsage : i32;
    enum class ImageLayout : i32;
}

namespace rk::utl {
    constexpr bool VALIDATION_LAYERS_ENABLED = true;
    constexpr i32 FRAMES_COUNT = 3;

    // validate layers required by this application
    constexpr const char* validateLayerNames[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    // device extensions required by this application
    constexpr const char* deviceExtensions[] = {
        "VK_KHR_swapchain" // VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    extern u32 findMemoryType(u32 type, MemoryType memoryType);


    extern void createBuffer(u64 size, VkBuffer& buffer, VkDeviceMemory& memory, BufferUsage usage, MemoryType memoryType);
    extern void copyBuffer(VkBuffer src, VkBuffer dst, u64 size);
    extern void copyDataToStagingBuffer(u64 size, VkDeviceMemory memoryType, const void* data);
    extern void copyBufferToImage(u32 width, u32 height, VkBuffer buffer, VkImage image);

    extern void createImage(u32 width, u32 height, VkImage& image, VkDeviceMemory& memory, Formats format, MemoryType memoryType, ImageUsage usageFlags);
    extern void transitionImageLayout(VkImage image, Formats format, ImageLayout oldLayout, ImageLayout newLayout);
    extern VkImageView createImageView(VkImage image, Formats format, u32 aspectFlag);
    
    extern VkCommandBuffer beginSingleTimeCommand();
    extern void endSingleTimeCommand(VkCommandBuffer command);
}
