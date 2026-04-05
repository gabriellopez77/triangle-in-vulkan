#pragma once

#include "render/DescriptorSet.h"
#include "render/GraphicsPipeline.h"
#include "render/Ubo.h"
#include "render/VertexBuffer.h"
#include "resources/ArrayBuffer.h"
#include "VerticesData.h"


namespace rk {
    class SpritesRenderer {
    public:
        resources::ArrayBuffer<SpriteVertices> buffer{MAX_SPRITES_COUNT};

        rk::Ubo ubo;
        rk::VertexBuffer vertexBuffer;
        rk::GraphicsPipeline pipeline;
        rk::DescriptorSet descriptorSet;

        void start();

        void draw();
    };
}
