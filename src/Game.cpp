#include "Game.h"

#include <iostream>

#include "Application.h"
#include "Inputs.h"

#include "render/PipelineSettings.h"
#include "render/core/VulkanApp.h"
#include "render/VulkanEnums.h"

#include "resources/ArrayBuffer.h"
#include "resources/TextureManager.h"

#include "math/Vec2.h"

struct VertexData {
    Vec2 vertices;
    Vec2 texCoords;
};

struct InstanceData {
    Vec3 position;
    Vec2 size;
};

constexpr VertexData vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {0.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f}}
};

constexpr u32 indices[] = {
    0, 1, 2, 2, 3, 0
};

//i32 uboIndex = 0;
i32 dynamicUboIndex = 0;
i32 dynamicUboOffsetIndex = 0;

constexpr int INSTANCE_COUNT = 5;

void Game::start(Application* application) {
    m_application = application;

    resources::textueManager::start();

    auto tex = resources::textueManager::get("sla");

    //uboIndex = descriptorSet.addUbo(128, 0, rk::ShaderStage::VERTEX);
    descriptorSet.addSampler(tex, 1, rk::ShaderStage::FRAGMENT);
    dynamicUboIndex = descriptorSet.addDynamicUbo();
    dynamicUboOffsetIndex = descriptorSet.addDynamicUboOffset(128);
    descriptorSet.createDynamicUbo(0, rk::ShaderStage::VERTEX);

    descriptorSet.create();

    rk::PipelineSettings pipelineSettings;
    pipelineSettings.cullMode = rk::CullMode::DISABLE;
    pipelineSettings.enableBlending = true;
    pipelineSettings.enableDepthTest = true;

    pipelineSettings.setShaders(SHADERS_FOLDER"/vertex.spv", SHADERS_FOLDER"/fragment.spv");
    pipelineSettings.addDynamicState(rk::DynamicState::VIEWPORT);
    pipelineSettings.addDynamicState(rk::DynamicState::SCISSOR);

    pipelineSettings.addBindings(0, rk::VertexInputRate::VERTEX, sizeof(VertexData));
    pipelineSettings.addAttributes(0, rk::Formats::RG_F32, offsetof(VertexData, vertices));
    pipelineSettings.addAttributes(1, rk::Formats::RG_F32, offsetof(VertexData, texCoords));

    pipelineSettings.addBindings(1, rk::VertexInputRate::INSTANCE, sizeof(InstanceData));
    pipelineSettings.addAttributes(2, rk::Formats::RGB_F32, offsetof(InstanceData, position));
    pipelineSettings.addAttributes(3, rk::Formats::RG_F32, offsetof(InstanceData, size));

    pipelineSettings.addDescriptorSet(descriptorSet);
    pipelineSettings.addPushConstant(0, 64, rk::ShaderStage::VERTEX);

    pipeline.create(pipelineSettings);

    InstanceData instanceData[INSTANCE_COUNT] = {
        {{0,  29,  30}, {100, 100} },
        {{20, 0,   10}, {130, 56} },
        {{0,  99, -10}, {150, 20} },
        {{40, 42,  10}, {10,  90} },
        {{34, 45,  50}, {100, 80} },
    };

    vertexBuffer1.create(sizeof(vertices), vertices, sizeof(indices), indices);
    vertexBuffer2.create(sizeof(instanceData), instanceData);
}

void Game::update(float dt) {
    player.update(dt);
}

void Game::render() {
    auto command = rk::vulkanApp::getCurrentCommandBuffer();

    pipeline.bind(command);
    vertexBuffer1.bind(command, 0);
    vertexBuffer2.bind(command, 1);
    descriptorSet.bind(command, pipeline.getLayout());

    const auto& proj = player.camera.getProjectionMatrix();
    const auto& view = player.camera.getViewMatrix();

    descriptorSet.enableDynamicUboOffset(dynamicUboIndex, dynamicUboOffsetIndex);
    descriptorSet.updateDynamicUbo(dynamicUboIndex, dynamicUboOffsetIndex, 0, 64, &proj);
    descriptorSet.updateDynamicUbo(dynamicUboIndex, dynamicUboOffsetIndex, 64, 64, &view);

    Matrix4 model(1.f);
    model.rotate(Application::Time * 25.f, {1.f, 0.f, 0.f});
    model.translate({0, 0, -1});
    model.scale({100, 100, 100});

    pipeline.bindPushConstant(command, 64, &model);
    vkCmdDrawIndexed(command, std::size(indices), INSTANCE_COUNT, 0, 0, 0);

    model = Matrix4(1.f);
    model.translate(Vec3(0.f, 0.f, 1));
    model.scale(Vec3(10.f, 10.f, 10.f));

    pipeline.bindPushConstant(command, 64, &model);
    vkCmdDrawIndexed(command, std::size(indices), INSTANCE_COUNT, 0, 0, 0);

    inputs::setMousePos(inputs::getMousePos());
}

void Game::resize(int width, int height) {
    player.camera.resize((float)width, (float)height);
}