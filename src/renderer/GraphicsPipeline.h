#pragma once

#include "VulkanFwd.h"

#include <vector>



namespace rk {
    //fwd
    class SwapChain;
    class RenderPass;
    class VulkanApp;
    struct PipelineSettings;

    class GraphicsPipeline {
    public:
        void create(const VulkanApp* app, const PipelineSettings& settings);
        void bind(VkCommandBuffer command) const;

        VkPipeline get() const { return m_graphicsPipeline; }
        VkPipelineLayout getLayout() const { return m_pipelineLayout; }

    private:
        std::vector<char> readShaderFile(const char* filePath) const;
        VkShaderModule createShaderModule(const char* path, VkDevice device) const;

        VkPipelineLayout m_pipelineLayout = nullptr;
        VkPipeline m_graphicsPipeline = nullptr;
    };
}
