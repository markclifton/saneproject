#include "saneengine/window.hpp"
#include <glad/glad.h>  // Include GLAD header
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace sane {
    Window::Window(const char* title, uint32_t width, uint32_t height)
        : mWidth(width), mHeight(height), mGLFWwindow(nullptr), mTitle(title)
    {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        mGLFWwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!mGLFWwindow) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(mGLFWwindow);
        glfwSwapInterval(1);
    }

    Window::~Window() {
        if (mGLFWwindow) {
            glfwDestroyWindow(mGLFWwindow);
        }
        glfwTerminate();
    }

    void Window::initializeGlad() {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(mGLFWwindow);
    }

    void Window::close() {
        glfwSetWindowShouldClose(mGLFWwindow, GLFW_TRUE);
    }

    void Window::swapBuffers() {
        glfwSwapBuffers(mGLFWwindow);
    }

    GLFWwindow* Window::getNativeWindow() const {
        return mGLFWwindow;
    }

} // namespace sane