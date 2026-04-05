#include "DescriptorSet.h"

#include <cassert>
#include <memory>

#include <vulkan/vulkan.h>

#include "core/VulkanApp.h"
#include "Ubo.h"
#include "Texture.h"
#include "math/Math.h"


void rk::DescriptorSet::create() {
    auto logicalDevice = vulkanApp::getLogicalDevice();

    std::vector<VkDescriptorSetLayoutBinding> layoutsBinding;
    layoutsBinding.reserve(m_layouts.size());
    
    for (const auto& layoutInfo : m_layouts) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = layoutInfo.binding;
        layoutBinding.descriptorCount = 1;
        layoutBinding.descriptorType = (VkDescriptorType)layoutInfo.type;
        layoutBinding.stageFlags = (u32)layoutInfo.shaderStage;

        layoutsBinding.push_back(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = (u32)layoutsBinding.size();
    layoutInfo.pBindings = layoutsBinding.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
        assert(false && "failed to create descriptor set layout!");

    VkDescriptorSetLayout layouts[utl::FRAMES_COUNT];

    for (auto& layout : layouts)
        layout = m_descriptorSetLayout;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = vulkanApp::getDescriptorPool();
    allocInfo.descriptorSetCount = utl::FRAMES_COUNT;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, m_descriptorSets) != VK_SUCCESS)
        assert(false && "failed to allocate descriptor set!");


    std::vector<VkWriteDescriptorSet> descriptorWrites;
    std::vector<std::unique_ptr<VkDescriptorBufferInfo>> buffersInfo;
    std::vector<std::unique_ptr<VkDescriptorImageInfo>> imagesInfo;

    for (int i = 0; i < utl::FRAMES_COUNT; i++) {
        for (const auto& info : m_layouts) {
            VkWriteDescriptorSet writer{};
            writer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writer.dstSet = m_descriptorSets[i];
            writer.dstBinding = info.binding;
            writer.descriptorType = (VkDescriptorType)info.type;
            writer.descriptorCount = 1;

            if (info.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || info.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
                // create in heap because it is passed by reference
                auto bufferInfo = std::make_unique<VkDescriptorBufferInfo>();
                bufferInfo->offset = 0;

                Ubo* buffer = nullptr;
                
                if (info.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                    buffer = &m_ubos[info.uboIndex];
                else
                    buffer = &m_dynamicUbos[info.uboIndex];

                bufferInfo->buffer = buffer->getBuffer(i);
                bufferInfo->range = buffer->getSize();

                writer.pBufferInfo = bufferInfo.get();

                buffersInfo.push_back(std::move(bufferInfo));
            }

            if (info.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                // create in heap because it is passed by reference
                auto imageInfo = std::make_unique<VkDescriptorImageInfo>();
                imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo->imageView = info.texture->getImageView();
                imageInfo->sampler = info.texture->getSampler();

                writer.pImageInfo = imageInfo.get();

                imagesInfo.push_back(std::move(imageInfo));
            }

            descriptorWrites.push_back(writer);

        }
  
        vkUpdateDescriptorSets(logicalDevice, (u32)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

        // clean for the next loop iteration
        descriptorWrites.clear();
        buffersInfo.clear();
        imagesInfo.clear();
    }
}

void rk::DescriptorSet::bind(VkCommandBuffer command, VkPipelineLayout pipelineLayout) {
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
        &m_descriptorSets[vulkanApp::getCurrentFrame()], (u32)m_offsets.size(), m_offsetsMask.data());

    // reset offsets mask
    for (auto i = 0; i < m_offsetsMask.size(); i++)
        m_offsetsMask[i] = 0;
}

void rk::DescriptorSet::addSampler(const Texture* texture, u32 binding, ShaderStage stage) {
    LayoutInfo layout{};
    layout.texture = texture;
    layout.binding = binding;
    layout.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout.shaderStage = stage;

    m_layouts.push_back(layout);
}

i32 rk::DescriptorSet::addUbo(u64 size, u32 binding, ShaderStage stage) {
    i32 index = m_ubos.size();

    LayoutInfo layout{};
    layout.uboIndex = index;
    layout.binding = binding;
    layout.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout.shaderStage = stage;

    m_layouts.push_back(layout);

    rk::Ubo ubo;
    ubo.create(size);

    m_ubos.push_back(ubo);

    return index;
}

i32 rk::DescriptorSet::addDynamicUbo() {
    m_dynamicUboOffset = 0;
    m_dynamicUboIndex = m_dynamicUbos.size();

    m_offsets.push_back({});
    m_offsetsMask.push_back(0);

    return m_dynamicUboIndex;
}

u32 rk::DescriptorSet::addDynamicUboOffset(u64 size) {
    u32 offset = m_dynamicUboOffset;

    const auto alignedSize = vulkanApp::getProperties().limits.minUniformBufferOffsetAlignment;
    //m_dynamicUboOffset += math::alignUp(size, d);
    m_dynamicUboOffset += (size + alignedSize - 1) & ~(alignedSize - 1);

    m_offsets[m_dynamicUboIndex].push_back(offset);

    return offset;
}

void rk::DescriptorSet::updateDynamicUbo(i32 dynamicUboIndex, i32 dynamicUboOffsetIndex, u32 additionalOffset, u64 size, const void* data) {
    u32 offset = m_offsets[dynamicUboIndex][dynamicUboOffsetIndex];

    assert(math::isAligned(offset, (u32)vulkanApp::getProperties().limits.minUniformBufferOffsetAlignment));

    m_dynamicUbos[dynamicUboIndex].update(offset + additionalOffset, size, data);
}

void rk::DescriptorSet::createDynamicUbo(u32 binding, ShaderStage stage) {
    i32 index = m_dynamicUbos.size();

    LayoutInfo layout{};
    layout.uboIndex = index;
    layout.binding = binding;
    layout.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    layout.shaderStage = stage;

    m_layouts.push_back(layout);

    rk::Ubo ubo;
    ubo.create(m_dynamicUboOffset);

    m_dynamicUbos.push_back(ubo);
}