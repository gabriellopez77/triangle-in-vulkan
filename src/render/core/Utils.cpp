#include "Utils.h"

#include <cassert>
#include <cstring>

#include <vulkan/vulkan.h>

#include "VulkanApp.h"
#include "../VulkanEnums.h"


namespace rk::utl {
    u32 findMemoryType(u32 type, MemoryType memoryType) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vulkanApp::getPhysicalDevice(), &memProperties);

        for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if (type & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & (u32)memoryType) == (u32)memoryType) {
                return i;
            }
        }

        assert(false && "failed to find suitable memory type!");

        return UINT32_MAX;
    }

    void copyDataToStagingBuffer(u64 size, VkDeviceMemory memoryType, const void* data) {
        auto logicalDevice = vulkanApp::getLogicalDevice();

        void* dataPtr;
        vkMapMemory(logicalDevice, memoryType, 0, size, 0, &dataPtr);
        std::memcpy(dataPtr, data, size);
        vkUnmapMemory(logicalDevice, memoryType);
    }

    void createBuffer(u64 size, VkBuffer& buffer, VkDeviceMemory& memory, BufferUsage usage, MemoryType memoryType) {
        auto logicalDevice = vulkanApp::getLogicalDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = (u32)usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // create buffer
        if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
            assert(false && "failed to create buffer!");

        // get memory requirements
        VkMemoryRequirements requirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &requirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = requirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(requirements.memoryTypeBits, memoryType);

        // allocate buffer memory
        if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS)
            assert(false && "failed to allocate buffer memory!");

        // bind memory to buffer
        vkBindBufferMemory(logicalDevice, buffer, memory, 0);
    }

    void copyBuffer(VkBuffer src, VkBuffer dst, u64 size) {
        auto commandBuffer = beginSingleTimeCommand();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

        endSingleTimeCommand(commandBuffer);
    }

    void createImage(u32 width, u32 height, VkImage& image, VkDeviceMemory& memory, Formats format,
        MemoryType memoryType, ImageUsage usageFlags) {
        auto logicalDevice = vulkanApp::getLogicalDevice();

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = (VkFormat)format;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = (u32)usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS)
            assert(false && "failed to create image!");


        // alloc memory for image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = utl::findMemoryType(memRequirements.memoryTypeBits, memoryType);

        if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS)
            assert(false && "failed to allocate image memory!");

        vkBindImageMemory(logicalDevice, image, memory, 0);
    }

    VkCommandBuffer beginSingleTimeCommand() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandPool = vulkanApp::getTransferCommandPool();

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vulkanApp::getLogicalDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommand(VkCommandBuffer command) {
        vkEndCommandBuffer(command);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &command;

        vkQueueSubmit(vulkanApp::getTransferQueue(), 1, &submitInfo, nullptr);
        vkQueueWaitIdle(vulkanApp::getTransferQueue());

        vkFreeCommandBuffers(vulkanApp::getLogicalDevice(), vulkanApp::getTransferCommandPool(), 1, &command);
    }

    void transitionImageLayout(VkImage image, Formats format, ImageLayout oldLayout, ImageLayout newLayout) {
        auto commandBuffer = beginSingleTimeCommand();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = (VkImageLayout)oldLayout;
        barrier.newLayout = (VkImageLayout)newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == ImageLayout::UNDEFINED && newLayout == ImageLayout::TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == ImageLayout::TRANSFER_DST_OPTIMAL && newLayout == ImageLayout::SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else assert(false && "unsupported layout transition!");

        vkCmdPipelineBarrier(
        commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommand(commandBuffer);
    }

    void copyBufferToImage(u32 width, u32 height, VkBuffer buffer, VkImage image) {
        auto commandBuffer = beginSingleTimeCommand();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = { width, height,1 };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommand(commandBuffer);
    }

    VkImageView createImageView(VkImage image, Formats format, u32 aspectFlag) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = (VkFormat)format;
        createInfo.subresourceRange.aspectMask = aspectFlag;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(vulkanApp::getLogicalDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS)
            assert(false && "failed to create image views!");

        return imageView;
    }
}
