#include "Utils.h"

#include "VulkanApp.h"
#include "defs.h"

#include <glfw/glfw3.h>


namespace rk::utils {
    std::vector<const char*> getRequiredInstanceExtensions() {
        std::vector<const char*> extensions;

        u32 glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        for (int i = 0; i < glfwExtensionCount; i++)
            extensions.push_back(glfwExtensions[i]);

        if (VulkanApp::VALIDATION_LAYERS_ENABLED)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }
}
