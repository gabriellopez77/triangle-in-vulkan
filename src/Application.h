#pragma once

#include "defs.h"

struct GLFWwindow;
class VulkanApp;

class Application {
public:
    static f32 Time;
    static f32 DeltaTime;

    void initWindow(i32 width, i32 height, const char* title);
    void initVulkan();
    void run();

    void setLoopFunc(void (*loopFunc)(float dt)) { m_loopFunc = loopFunc; }

private:   
    VulkanApp* m_vulkanApp = nullptr;

    // main loop function pointer
    void (*m_loopFunc)(float dt) = nullptr;

    // glfw window pointer
    GLFWwindow* m_window = nullptr;
    i32 m_windowWidth = 0;
    i32 m_windowHeight = 0;

    // last frame time
    f32 m_lastFrame = 0.f;
};