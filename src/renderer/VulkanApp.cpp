#include "VulkanApp.h"

#include <GLFW/glfw3.h>

#include <fstream>
#include <cassert>

#include "Utils.h"
#include "PipelineSettings.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>


struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributes;

        attributes[0].location = 0;
        attributes[0].binding = 0;
        attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, pos);

        attributes[1].location = 1;
        attributes[1].binding = 0;
        attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, color);

        return attributes;
    }
};

const Vertex vertices[] = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const uint16_t indices[] = {
    0, 1, 2, 2, 3, 0
};

void rk::VulkanApp::init(GLFWwindow* window) {
    this->window = window;

    createInstance();

    swapChain.createSurface(this, window);
    physicalDevice.create(this); // pick physical device
    logicalDevice.create(this); // create logical device
    swapChain.create(this); // create swapChain
    swapChain.createImageViews(this);
    renderPass.create(this); // create renderPass

    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    pushConstants.create(sizeof(glm::mat4) * 2, VK_SHADER_STAGE_VERTEX_BIT);

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.size = sizeof(glm::mat4) * 2;
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.stride = sizeof(Vertex);

    // dynamic states
    constexpr VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    PipelineSettings pipelineSettings;
    pipelineSettings.vertPath = SHADERS_FOLDER"/vertex1.spv";
    pipelineSettings.fragPath = SHADERS_FOLDER"/fragment1.spv";
    pipelineSettings.attributesCount = attributeDescriptions.size();
    pipelineSettings.attributes = attributeDescriptions.data();
    pipelineSettings.bindingsCount = 1;
    pipelineSettings.bindings = &binding;
    pipelineSettings.dynamicStatesCount = 2;
    pipelineSettings.dynamicStates = dynamicStates;
    pipelineSettings.pushConstantsCount = 1;
    pipelineSettings.pushConstants = &pushConstants;

    graphicsPipeline1.create(this, pipelineSettings); // create graphics pipeline 1
    swapChain.createFramebuffers(this); // create framebuffers

    createCommandPool();
    vertexBuffer.createVertexBuffer(this, sizeof(vertices[0]) * std::size(vertices), vertices);
    vertexBuffer.createIndexBuffer(this, sizeof(indices[0]) * std::size(indices), indices, VK_INDEX_TYPE_UINT16);
    createCommandBuffers();
    createSyncObjects();
}

void rk::VulkanApp::clear() {
    vkDeviceWaitIdle(logicalDevice.get());

    swapChain.clear(this);
}

void rk::VulkanApp::resize() {
    swapChain.needRecreate();
}

void rk::VulkanApp::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // get required extensions
    auto requiredExtensions = utils::getRequiredInstanceExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // validation layer
    if (VALIDATION_LAYERS_ENABLED) {
        createInfo.enabledLayerCount = std::size(utils::validateLayerNames);
        createInfo.ppEnabledLayerNames = utils::validateLayerNames;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        assert(false && "Failed to create Vulkan instance!");
}

rk::VulkanApp::QueueFamilyIndices rk::VulkanApp::findQueueFamilies(VkPhysicalDevice device, const std::vector<VkQueueFamilyProperties>* queues) const {
    auto* queuesFamilies = queues == nullptr ? physicalDevice.getQueueFamiliesCache() : queues;

    // try to find a queue family that supports graphics
    QueueFamilyIndices indices;
    for (int i = 0; i < queuesFamilies->size(); i++) {
        if ((*queuesFamilies)[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if ((*queuesFamilies)[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            indices.transferFamily = i;
        }

        // check if the queue family supports presentation to our window surface
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, swapChain.getSurface(), &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
            break;
    }

    return indices;
}

void rk::VulkanApp::createCommandPool() {
    auto queueFamilyIndices = findQueueFamilies(physicalDevice.get());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(logicalDevice.get(), &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        assert(false && "failed to create command pool!");
}

void rk::VulkanApp::createCommandBuffers() {
    m_commandBuffers.resize(swapChain.getImagesCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = swapChain.getImagesCount();


    if (vkAllocateCommandBuffers(logicalDevice.get(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        assert(false && "failed to allocate command buffers!");
}

void rk::VulkanApp::createSyncObjects() {
    m_imageAvailableSemaphore.resize(swapChain.getImagesCount());
    m_renderFinishedSemaphore.resize(swapChain.getImagesCount());
    m_inFlightFence.resize(swapChain.getImagesCount());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < swapChain.getImagesCount(); i++) {
        if (vkCreateSemaphore(logicalDevice.get(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphore[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice.get(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS ||
            vkCreateFence(logicalDevice.get(), &fenceInfo, nullptr, &m_inFlightFence[i]) != VK_SUCCESS) {
            assert(false && "failed to create semaphores!");
        }
    }
}

glm::mat4 model(1.f);
void rk::VulkanApp::drawFrame() {
    auto commandBuffer = m_commandBuffers[m_currentFrame];

    graphicsPipeline1.bind(commandBuffer);
    vertexBuffer.bind(commandBuffer);

    float time = (float)glfwGetTime();

    model = glm::translate(glm::mat4(1.f), glm::vec3(swapChain.getWidth() / 2.f, swapChain.getHeight() / 2.f, 0.f));
    model = glm::rotate(model,  glm::radians(time) * 20.f, glm::vec3(0.f, 0.f, 1.f));
    model = glm::scale(model, glm::vec3(100.f));
    glm::mat4 matrices[2] = {
        glm::ortho(0.f, (float)swapChain.getWidth(), 0.f, (float)swapChain.getHeight(), -10.f, 10.f),
        model
    };

    pushConstants.bind(commandBuffer, graphicsPipeline1.getLayout(), sizeof(matrices), matrices);
    vkCmdDrawIndexed(commandBuffer, std::size(indices), 1, 0, 0, 0);
}

void rk::VulkanApp::beginFrame() {
    // saves a copy in stack because it is faster
    auto currentFrame = m_currentFrame;

    // wait for previous frame and reset fence state
    vkWaitForFences(logicalDevice.get(), 1, &m_inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(logicalDevice.get(), 1, &m_inFlightFence[currentFrame]);

    u32 imageIndex = swapChain.getOneImage(logicalDevice.get(), m_imageAvailableSemaphore[currentFrame]);


    constexpr VkClearValue clearColor = { .color = { .float32 = { 0.f, 0.f, 0.f, 1.f } } };

    auto commandBuffer = m_commandBuffers[currentFrame];

    // rest command buffer
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        assert(false && "failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass.get();
    renderPassInfo.framebuffer = swapChain.getFramebuffer(imageIndex);
    renderPassInfo.renderArea.extent = swapChain.getSize();
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


    // set dynamic states
    VkViewport viewport{};
    viewport.width = swapChain.getWidth();
    viewport.height = swapChain.getHeight();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.extent = swapChain.getSize();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void rk::VulkanApp::endFrame() {
    // saves a copy in stack because it is faster
    auto currentFrame = m_currentFrame;
    auto commandBuffer = m_commandBuffers[currentFrame];
    u32 imageIndex = swapChain.getImageIndex();

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        assert(false && "failed to record command buffer!");


    const auto signalSemaphores = m_renderFinishedSemaphore[currentFrame];
    constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore[currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphores;

    if (vkQueueSubmit(logicalDevice.getGraphicsQueue(), 1, &submitInfo, m_inFlightFence[currentFrame]) != VK_SUCCESS)
        assert(false && "failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain.getSwapChain();
    presentInfo.pImageIndices = &imageIndex;

    const auto result = vkQueuePresentKHR(logicalDevice.getPresentQueue(), &presentInfo);

    // recreate swap chain if necessary
    swapChain.tryRecreate(this, result, &m_inFlightFence[currentFrame]);

    //advance to the next frame
    currentFrame = (currentFrame + 1) % swapChain.getImagesCount();
    m_currentFrame = currentFrame;
}