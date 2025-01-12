#pragma once

#include "saneengine/utils/api.hpp"
#include "saneengine/layer/layer.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

namespace sane {
    class SANEENGINE_API ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override;

        void onAttach() override;
        void onDetach() override;
        void onUpdate(float deltaTime) override;
        void onRender() final override;

    private:
        class Impl;
        Impl* mImpl;
    };
}