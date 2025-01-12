#pragma once
#include "saneengine/layer/imguilayer.hpp"

namespace sane {
    class SANEENGINE_API ImGuiPerformanceLayer : public ImGuiLayer {
    public:
        ImGuiPerformanceLayer(const char* name = "ImGui Performance Layer", int32_t priority = -1);
        ~ImGuiPerformanceLayer() override;

        void onUpdate(float deltaTime) override;

    private:
        class Impl;
        Impl* mImpl;
    };
}