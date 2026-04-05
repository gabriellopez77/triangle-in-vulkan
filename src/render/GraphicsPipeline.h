#pragma once

#include "Defs.h"

#include "core/VulkanFwd.h"


namespace rk {
    //fwd
    class SwapChain;
    class PipelineSettings;
    enum class ShaderStage : i32;

    class GraphicsPipeline {
    public:
        void create(const PipelineSettings& settings);
        void bind(VkCommandBuffer command) const;

        void bindPushConstant(VkCommandBuffer command, u64 size, const void* data) const;

        VkPipeline get() const { return m_graphicsPipeline; }
        VkPipelineLayout getLayout() const { return m_pipelineLayout; }

    private:
        bool m_usePushConstant = false;
        ShaderStage m_pushConstantShaderStage{};

        VkPipelineLayout m_pipelineLayout = nullptr;
        VkPipeline m_graphicsPipeline = nullptr;
    };
}
