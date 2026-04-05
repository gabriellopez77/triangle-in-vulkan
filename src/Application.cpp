#include "Application.h"

#include <GLFW/glfw3.h>

#include <cassert>

#include "render/core/VulkanApp.h"

#include "Game.h"
#include "inputs.h"


f32 Application::DeltaTime = 0.f;
f32 Application::Time = 0.f;

void Application::initWindow(i32 width, i32 height, const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    m_windowWidth = width;
    m_windowHeight = height;

    glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, resizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    setCursorMode(CursorMode::Disabled);
}

void Application::clear() {
    rk::vulkanApp::clear();
}

void Application::run(Game* game) {
    if (!m_window) {
        assert(false && "Window not initialized. Call initWindow() before run().");
    }

    m_game = game;

    // init vulkan
    rk::vulkanApp::init(this);

    // init game
    game->start(this);
    game->resize(m_windowWidth, m_windowHeight);

    while (!glfwWindowShouldClose(m_window)) {
        inputs::newFrame();

        // poll events
        glfwPollEvents();


        // calculate delta time
        float time = (f32)glfwGetTime();
        DeltaTime = time - m_lastFrame;
        m_lastFrame = time;
        Time = time;

        // run game loop
        game->update(DeltaTime);

        rk::vulkanApp::beginFrame();

        // run game render
        game->render();

        rk::vulkanApp::endFrame();
    }
}

void Application::setCursorMode(CursorMode mode) {
    glfwSetInputMode(m_window, GLFW_CURSOR, (int)mode);
}

void Application::getFrameBufferSize(i32* width, i32* height) const {
    glfwGetFramebufferSize(m_window, width, height);
}

void Application::resizeCallback(GLFWwindow* window, i32 width, i32 height) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    rk::vulkanApp::resize();

    app->m_game->resize(width, height);

    app->m_windowWidth = width;
    app->m_windowHeight = height;

}

void Application::keyCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) {
    if (key == GLFW_KEY_UNKNOWN)
        return;

    inputs::setKeyState(key, action != GLFW_RELEASE);
}

void Application::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    inputs::setMouseButtonState(button, action != GLFW_RELEASE);
}

void Application::cursorPosCallback(GLFWwindow* window, double x, double y) {
    inputs::setMousePos({ (float)x, (float)y });
}
