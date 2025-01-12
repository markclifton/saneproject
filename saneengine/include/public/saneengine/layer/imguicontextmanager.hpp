#pragma once
#include "saneengine/utils/api.hpp"
#include "saneengine/utils/notcopyable.hpp"

namespace sane {
    class SANEENGINE_API ImGuiContextManager : public utils::NotCopyable {
    public:
        ImGuiContextManager();
        ~ImGuiContextManager();

        void initialize();
        void shutdown();

        void beginFrame();
        void endFrame();

        bool isInitialized() const;

        void updateStats(float deltaTime);
        float getAverageFPS() const;
        float getOnePercentLow() const;
        float getPointOnePercentLow() const;

    private:
        class Impl;
        Impl* mImpl;
    };
}