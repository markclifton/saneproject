#pragma once
#include "saneengine/utils/api.hpp"
#include <cstdint>

struct GLFWwindow;

namespace sane {
    class SANEENGINE_API Window {
    public:
        Window(const char* title = "Sane Engine", uint32_t width = 800, uint32_t height = 600);
        ~Window();

        void initializeGlad();
        bool shouldClose() const;
        void close();
        void swapBuffers();
        GLFWwindow* getNativeWindow() const;

    private:
        GLFWwindow* mGLFWwindow;
        const char* mTitle;
        uint32_t mWidth;
        uint32_t mHeight;
    };
}