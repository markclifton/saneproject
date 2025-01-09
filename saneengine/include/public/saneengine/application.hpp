#pragma once

// STL headers
#include <cstdint>

// Public headers
#include "saneengine/api.hpp"

namespace sane {
    class SANEENGINE_API Application {
    public:
        Application(const char* title, uint32_t width, uint32_t height);
        virtual ~Application();
        void run();

        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}

    private:
        class WindowImpl;
        WindowImpl* mWindowImpl;

        void mainLoop();
        void setupRenderThread();
    };
}