#include "VulkanApp.h"

#include <cassert>
#include <unordered_set>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "Application.h"
#include "../VulkanEnums.h"


namespace rk::vulkanApp {
    // fwd
    void createInstance();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createRenderPass();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();
    void createDescriptorPool();

    const Application* application = nullptr;

    SwapChain swapChain;

    u32 lastFrame = 0;
    u32 currentFrame = 0;

    // commands
    VkDescriptorPool descriptorPool = nullptr;
    VkCommandPool transferCommandPool = nullptr;
    VkCommandPool graphicsCommandPool = nullptr;
    VkCommandBuffer commandBuffers[utl::FRAMES_COUNT] = {};

    // others
    VkInstance vkInstance = nullptr;
    VkPhysicalDevice physicalDevice = nullptr;
    VkDevice logicalDevice = nullptr;
    VkRenderPass renderPass = nullptr;
    VkPhysicalDeviceProperties physicalDeviceProperties;

    // queues
    VkQueue graphicsQueue = nullptr;
    VkQueue presentQueue = nullptr;
    VkQueue transferQueue = nullptr;

    // sync objects
    VkSemaphore imageAvailableSemaphore[utl::FRAMES_COUNT] = {};
    VkSemaphore renderFinishedSemaphore[utl::FRAMES_COUNT] = {};
    VkFence inFlightFence[utl::FRAMES_COUNT] = {};
    VkFence imagesInFlight[utl::FRAMES_COUNT] = {};


    u32 getCurrentFrame() { return currentFrame; }

    VkDevice getLogicalDevice() { return logicalDevice; }
    VkQueue getGraphicsQueue() { return graphicsQueue; }
    VkQueue getPresentQueue() { return presentQueue; }
    VkQueue getTransferQueue() { return transferQueue; }

    VkRenderPass getRenderPass() { return renderPass; }

    const VkPhysicalDeviceProperties& getProperties() { return physicalDeviceProperties; }
    VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
    VkInstance getVkInstance() { return vkInstance; }

    VkCommandBuffer getCurrentCommandBuffer() { return commandBuffers[currentFrame]; }
    VkDescriptorPool getDescriptorPool() { return descriptorPool; }
    VkCommandPool getTransferCommandPool() { return transferCommandPool; }


    void init(const Application* application) {
        vulkanApp::application = application;

        createInstance();

        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(vkInstance, application->getWindow(), nullptr, &surface) != VK_SUCCESS)
            assert(false && "failed to create window surface!");

        swapChain.setSurface(surface);
        pickPhysicalDevice();
        createLogicalDevice();
        swapChain.create();
        createRenderPass();
        swapChain.createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        createDescriptorPool();

        // get physical device roperties
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        size_t alignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;
    }

    void clear() {
        vkDeviceWaitIdle(logicalDevice);

        swapChain.clear();
    }

    void resize() {
        int width, height;
        glfwGetFramebufferSize(application->getWindow(), &width, &height);

        // avoid create a framebuffer with 0 as size
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(application->getWindow(), &width, &height);
            glfwWaitEvents();
        }

        swapChain.recreate(inFlightFence[lastFrame], width, height);

        for (auto& fence : imagesInFlight)
            fence = nullptr;
    }

    void createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        // get required extensions
        std::vector<const char*> requiredExtensions;

        u32 glfwExtensionCount = 0;
        auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        requiredExtensions.reserve(glfwExtensionCount);

        for (u32 i = 0; i < glfwExtensionCount; i++)
            requiredExtensions.push_back(glfwExtensions[i]);

        if (utl::VALIDATION_LAYERS_ENABLED)
            requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);


        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
        createInfo.ppEnabledExtensionNames = requiredExtensions.data();

        // validation layer
        if (utl::VALIDATION_LAYERS_ENABLED) {
            createInfo.enabledLayerCount = std::size(utl::validateLayerNames);
            createInfo.ppEnabledLayerNames = utl::validateLayerNames;
        }

        if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
            assert(false && "Failed to create Vulkan instance!");
    }

    void pickPhysicalDevice() {
        auto isSuitable = [](VkPhysicalDevice device) -> bool {
            // check if device supports all required extensions
            u32 extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            bool extensionsSupported = false;
            for (auto extensionName : rk::utl::deviceExtensions) {

                for (const auto& extensionProperties : availableExtensions)
                    if (strcmp(extensionName, extensionProperties.extensionName))
                        extensionsSupported = true;

                if (!extensionsSupported)
                    return false;
            }


            // check if the device has a graphics queue family
            auto indices = findQueueFamilies(device);

            // if extensions are supported, check if the swap chain is adequate
            return indices.isComplete() && extensionsSupported && SwapChain::isAdequate(device, swapChain.getSurface());
        };

        // get physical devices count
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
            assert(false && "Failed to find GPUs with Vulkan support!");

        // get physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

        // check each device for suitability and choice the first one that is suitable
        for (u32 i = 0; i < deviceCount; i++) {
            if (isSuitable(devices[i])) {
                physicalDevice = devices[i];
                break;
            }
        }

        if (physicalDevice == nullptr)
            assert(false && "Failed to find a suitable GPU!");
    }

    void createLogicalDevice() {
        // grab queue family indices
        auto indices = findQueueFamilies();

        // create device queue create info
        const f32 queuePriority = 1.0f;

        // we need to create a queue for each unique queue family (we use a std::set why graphics and present can have the same index)
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::unordered_set<u32> queueFamilies = { indices.graphics.value(),indices.present.value(), indices.transfer.value() };

        for (u32 queueFamily : queueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = std::size(utl::deviceExtensions);
        createInfo.ppEnabledExtensionNames = utl::deviceExtensions;

        // create the logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS)
            assert(false && "failed to create logical device!");

        // get graphics and present queues
        vkGetDeviceQueue(logicalDevice, indices.graphics.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, indices.present.value(), 0, &presentQueue);
        vkGetDeviceQueue(logicalDevice, indices.transfer.value(), 0, &transferQueue);
    }

    void createRenderPass() {
            // color attachment
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[2] = {};

        // color
        attachments[0].format = (VkFormat)swapChain.getImageFormat();
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // depth
        attachments[1].format = (VkFormat)Formats::DEPTH_F32;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = std::size(attachments);
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            assert(false && "failed to create render pass!");
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        auto physicalDevice = device == nullptr ? vulkanApp::physicalDevice : device;

        // get queue family properties count
        u32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        // get queue family properties
        std::vector<VkQueueFamilyProperties> queues(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queues.data());


        QueueFamilyIndices indices;

        // graphics
        for (int i = 0; i < queues.size(); i++) {
            u32 flags = queues[i].queueFlags;

            if (flags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics = i;
                break;
            }
        }

        assert(indices.graphics.has_value());

        // present
        for (int i = 0; i < queues.size(); i++) {
            u32 flags = queues[i].queueFlags;

            // try to get present queue that not contains graphics bit
            if ((flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, swapChain.getSurface(), &presentSupport);

                if (presentSupport) {
                    indices.present = i;
                    break;
                }
            }
        }
        if (!indices.present.has_value())
            indices.present = indices.graphics;

        // transfer
        for (int i = 0; i < queues.size(); i++) {
            u32 flags = queues[i].queueFlags;

            // try to get transfer queue that not contains graphics bit
            //if (flags & VK_QUEUE_TRANSFER_BIT && (flags & VK_QUEUE_GRAPHICS_BIT) == 0) {
            if (flags & VK_QUEUE_TRANSFER_BIT) {
                indices.transfer = i;
                break;
            }
        }
        if (!indices.transfer.has_value())
            indices.transfer = indices.graphics;

        return indices;
    }

    void createCommandPool() {
        auto queueFamilyIndices = findQueueFamilies();

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.transfer.value();

        if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &transferCommandPool) != VK_SUCCESS)
            assert(false && "failed to create command pool!");

        VkCommandPoolCreateInfo poolInfo2{};
        poolInfo2.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo2.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo2.queueFamilyIndex = queueFamilyIndices.graphics.value();

        if (vkCreateCommandPool(logicalDevice, &poolInfo2, nullptr, &graphicsCommandPool) != VK_SUCCESS)
            assert(false && "failed to create command pool!");
    }

    void createCommandBuffers() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = graphicsCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = utl::FRAMES_COUNT;


        if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers) != VK_SUCCESS)
            assert(false && "failed to allocate command buffers!");
    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < utl::FRAMES_COUNT; i++) {
            if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
                vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
                vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS) {
                assert(false && "failed to create semaphores!");
            }
        }
    }

    void createDescriptorPool() {
        constexpr u32 maxSetsInDescriptors = 10;
        constexpr u32 maxDescriptorCount = 10;

        VkDescriptorPoolSize poolSizes[] = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, utl::FRAMES_COUNT * maxDescriptorCount },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, utl::FRAMES_COUNT * maxDescriptorCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, utl::FRAMES_COUNT * maxDescriptorCount },
        };
        
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = std::size(poolSizes);
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = maxSetsInDescriptors;

        if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
            assert(false && "failed to create descriptor pool!");
    }

    void beginFrame() {
        // wait for previous frame
        vkWaitForFences(logicalDevice, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

        // get next image from swapChain
        u32 imageIndex = swapChain.getOneImage(logicalDevice, imageAvailableSemaphore[currentFrame]);

        if (imagesInFlight[imageIndex] != nullptr)
            vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

        imagesInFlight[swapChain.getImageIndex()] = inFlightFence[currentFrame];

        // reset fence state
        vkResetFences(logicalDevice, 1, &inFlightFence[currentFrame]);


        auto commandBuffer = commandBuffers[currentFrame];

        // reset command buffer
        vkResetCommandBuffer(commandBuffer, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            assert(false && "failed to begin recording command buffer!");

        const VkClearValue clearValues[2] = {
            { .color = {{0.0f, 0.0f, 0.0f, 1.0f}} },
            { .depthStencil = {1.0f, 0} }
        };

        VkExtent2D extent = { swapChain.getWidth(), swapChain.getHeight() };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChain.getFramebuffer(imageIndex);
        renderPassInfo.renderArea.extent = extent;
        renderPassInfo.clearValueCount = std::size(clearValues);
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // set dynamic states
        VkRect2D scissor{};
        scissor.extent = extent;

        VkViewport viewport{};
        viewport.width = extent.width;
        viewport.height = extent.height;
        viewport.maxDepth = 1.0f;

        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void endFrame() {
        auto commandBuffer = commandBuffers[currentFrame];

        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            assert(false && "failed to record command buffer!");


        const auto signalSemaphore = renderFinishedSemaphore[currentFrame];
        constexpr VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphore[currentFrame];
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signalSemaphore;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence[currentFrame]) != VK_SUCCESS)
            assert(false && "failed to submit draw command buffer!");

        // the same image index obtained from getOneImage in begin frame
        u32 imageIndex = swapChain.getImageIndex();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &signalSemaphore;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain.get();
        presentInfo.pImageIndices = &imageIndex;

        vkQueuePresentKHR(presentQueue, &presentInfo);

        //advance to the next frame
        lastFrame = currentFrame;
        currentFrame = (currentFrame + 1) % utl::FRAMES_COUNT;
    }
}