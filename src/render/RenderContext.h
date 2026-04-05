#pragma once

#include "GraphicsPipeline.h"
#include "DescriptorSet.h"
#include "Texture.h"


namespace rk {
    class RenderContext {
    public:
        void create(const PipelineSettings& pipelineSettings);
        void destroy();
        void draw();

        GraphicsPipeline pipeline;
        DescriptorSet descriptorSet;
        const Texture* texture = nullptr;
    };
}