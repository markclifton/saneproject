#pragma once

#include "saneengine/utils/notcopyable.hpp" 

namespace sane {

    class ImGuiContext : utils::NotCopyable {
    public:
        static ImGuiContext& get();

        void incrementRef();
        void decrementRef();
        bool isInitialized() const;

        void updateStats(float deltaTime);
        float getAverageFPS() const;
        float getOnePercentLow() const;
        float getPointOnePercentLow() const;

        void beginFrame(float deltaTime);
        void endFrame();

    private:
        ImGuiContext();
        ~ImGuiContext();

        void initialize();
        void shutdown();

        class Impl;
        Impl* mImpl;
    };
}