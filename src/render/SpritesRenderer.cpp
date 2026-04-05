#include "SpritesRenderer.h"

#include "PipelineSettings.h"
#include "VulkanEnums.h"


void rk::SpritesRenderer::start() {
    ubo.create(128);

    vertexBuffer.create(sizeof(SPRITES_VERTICES), SPRITES_VERTICES, sizeof(SPRITES_INDICES), SPRITES_INDICES);

    //descriptorSet.addUbo(ubo, 0, rk::ShaderStage::VERTEX);
    //descriptorSet.addSampler(*tex, 1, rk::ShaderStage::FRAGMENT);
    //descriptorSet.create();

    rk::PipelineSettings pipelineSettings;
    pipelineSettings.cullMode = rk::CullMode::DISABLE;
    pipelineSettings.enableBlending = true;
    pipelineSettings.enableDepthTest = true;

    pipelineSettings.setShaders(SHADERS_FOLDER"/vertex.spv", SHADERS_FOLDER"/fragment.spv");
    pipelineSettings.addDynamicState(rk::DynamicState::VIEWPORT);
    pipelineSettings.addDynamicState(rk::DynamicState::SCISSOR);

    pipelineSettings.addBindings(0, rk::VertexInputRate::VERTEX, sizeof(f32) * 4);
    pipelineSettings.addAttributes(0, rk::Formats::RG_F32, 0);
    pipelineSettings.addAttributes(1, rk::Formats::RG_F32, 2 * sizeof(f32));

    pipelineSettings.addDescriptorSet(descriptorSet);
    pipelineSettings.addPushConstant(0, 64, rk::ShaderStage::VERTEX);

    pipeline.create(pipelineSettings);
}

void rk::SpritesRenderer::draw() {

    u32 instancesCount = buffer.length();

    if (instancesCount == 0)
        return;

    //descriptorSet.bind
    ubo.updateAll(0, buffer.sizeInBytes(), buffer.data());
    vkCmdDrawIndexed(nullptr, std::size(rk::SPRITES_INDICES), instancesCount, 0, 0, 0);

    buffer.clear();
}