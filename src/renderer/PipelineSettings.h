#pragma once

#include "defs.h"


namespace rk {
    class PushConstants;

    struct PipelineSettings {
        const char* vertPath = nullptr;
        const char* fragPath = nullptr;

        u32 dynamicStatesCount = 0;
        const VkDynamicState* dynamicStates = nullptr;

        u32 bindingsCount = 0;
        const VkVertexInputBindingDescription* bindings = nullptr;

        u32 attributesCount = 0;
        const VkVertexInputAttributeDescription* attributes = nullptr;

        u32 pushConstantsCount = 0;
        const PushConstants* pushConstants = nullptr;
    };
}