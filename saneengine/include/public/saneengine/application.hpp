#pragma once

// STL headers
#include <cstdint>
#include <memory>

// Public headers
#include "saneengine/api.hpp"

namespace sane {
    class Layer;
    class LayerStack;

    class SANEENGINE_API Application {
    public:
        Application(const char* title, uint32_t width, uint32_t height);
        virtual ~Application();
        void run();

        void pushLayer(std::unique_ptr<Layer> layer);
        void popLayer();

    private:
        class Impl;
        Impl* mImpl;

        void mainLoop();
        void setupRenderThread();
    };
}