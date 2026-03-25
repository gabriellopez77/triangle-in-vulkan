#pragma once

#include "VulkanFwd.h"


namespace rk {
    //fwd
    class VulkanApp;

    class RenderPass {
    public:
        void create(const VulkanApp* app);

        VkRenderPass get() const { return m_renderPass; }

    private:
        VkRenderPass m_renderPass = nullptr;
    };
}
