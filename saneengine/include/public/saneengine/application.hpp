#pragma once

// STL headers
#include <cstdint>
#include <memory>

// Public headers
#include "saneengine/utils/api.hpp"
#include "saneengine/utils/notcopyable.hpp"
#include "saneengine/utils/api.hpp"
#include "saneengine/utils/uuid.hpp"

namespace sane {
    class Layer;
    namespace ecs {
        class System;
        class SystemManager;
    }

    class SANEENGINE_API Application : public utils::NotCopyable {
    public:
        Application(const char* name = "SaneApplication",
            uint32_t width = 1280,
            uint32_t height = 720);
        virtual ~Application();

        void run();
        void pushLayer(std::unique_ptr<Layer> layer);
        void popLayer();

        // System management
        utils::UUID startSystem(std::unique_ptr<ecs::System> system);
        void stopSystem(utils::UUID systemId);
        ecs::System* getSystem(utils::UUID systemId);
        ecs::SystemManager& getSystemManager();

    private:
        void mainLoop();
        void setupRenderThread();

        class Impl;
        Impl* mImpl;
    };
}