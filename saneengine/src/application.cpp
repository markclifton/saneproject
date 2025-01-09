#include "saneengine/application.hpp"
#include "saneengine/window.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>

namespace sane {
    class Application::WindowImpl {
    public:
        std::unique_ptr<Window> window;
        std::thread renderThread;
        std::atomic<bool> running{ true };
        std::chrono::steady_clock::time_point lastFrameTime;
    };

    Application::Application(const char* title, uint32_t width, uint32_t height)
        : mWindowImpl(new WindowImpl)
    {
        mWindowImpl->window = std::make_unique<Window>(title, width, height);
        setupRenderThread();
    }

    Application::~Application() {
        delete mWindowImpl;
    }

    void Application::run() {
        mainLoop();
    }

    void Application::mainLoop() {
        mWindowImpl->lastFrameTime = std::chrono::steady_clock::now();

        glfwMakeContextCurrent(nullptr);
        while (!mWindowImpl->window->shouldClose()) {
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - mWindowImpl->lastFrameTime).count();
            mWindowImpl->lastFrameTime = currentTime;

            glfwPollEvents();
            onUpdate(deltaTime);
        }

        mWindowImpl->running = false;
        if (mWindowImpl->renderThread.joinable()) {
            mWindowImpl->renderThread.join();
        }
    }

    void Application::setupRenderThread() {
        mWindowImpl->renderThread = std::thread([this] {
            glfwMakeContextCurrent(mWindowImpl->window->getNativeWindow());
            while (mWindowImpl->running) {
                onRender();
                mWindowImpl->window->swapBuffers();
                glClearColor(0.4f, 0.6f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }
            glfwMakeContextCurrent(nullptr);
            });
    }
}
