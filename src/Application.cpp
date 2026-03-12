#include "Application.h"

#include <GLFW/glfw3.h>

#include <cassert>

#include "VulkanApp.h"


f32 Application::Time = 0.f;
f32 Application::DeltaTime = 0.f;

void Application::initWindow(i32 width, i32 height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    m_windowWidth = width;
    m_windowHeight = height;
}

void Application::initVulkan() {
    if (m_vulkanApp)
        assert(false && "Vulkan app already initialized!");

    m_vulkanApp = new VulkanApp();
    m_vulkanApp->init(m_window);
}

void Application::run() {
    if (!m_window) {
        assert(false && "Window not initialized. Call initWindow() before run().");
    }

    while (!glfwWindowShouldClose(m_window)) {
        // poll events
        glfwPollEvents();

        // calculate delta time
        Time = (f32)glfwGetTime();
        DeltaTime = Time - m_lastFrame;
        m_lastFrame = Time;

        // run the main loop function if it's set
        if (m_loopFunc) {
            m_loopFunc(DeltaTime);
        }

        m_vulkanApp->drawFrame();
    }
}