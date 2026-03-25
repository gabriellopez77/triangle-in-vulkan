#pragma once

#include "renderer/defs.h"


// fwd
struct GLFWwindow;
namespace rk {
    class VulkanApp;
}

class Application {
public:
    static f32 Time;
    static f32 DeltaTime;

    void initWindow(i32 width, i32 height, const char* title);
    void initVulkan();
    void clearVulkan();
    void run();

    void setLoopFunc(void (*loopFunc)(float dt)) { m_loopFunc = loopFunc; }
    void setResizeFunc(void (*resizeFun)(i32 width, i32 height)) {m_resizeFunc = resizeFun; }

private:
    static void resizeCallback(GLFWwindow* window, i32 width, i32 height);

    rk::VulkanApp* m_vulkanApp = nullptr;

    // functions pointers
    void (*m_loopFunc)(float dt) = nullptr;
    void (*m_resizeFunc)(i32 width, i32 height) = nullptr;

    // glfw window pointer
    GLFWwindow* m_window = nullptr;
    i32 m_windowWidth = 0;
    i32 m_windowHeight = 0;

    // last frame time
    f32 m_lastFrame = 0.f;
};