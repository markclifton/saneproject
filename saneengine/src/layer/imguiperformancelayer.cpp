#include "saneengine/layer/imguiperformancelayer.hpp"
#include <imgui.h>
#include <vector>
#include <algorithm>

namespace sane {
    class ImGuiPerformanceLayer::Impl {
    public:
        static constexpr size_t MAX_SAMPLES = 100;
        std::vector<float> frameTimeHistory;
        float onePercentLow{ 0.0f };
        float pointOnePercentLow{ 0.0f };
        ImVec2 lastWindowPos{ 10.0f, 10.0f };

        void updateStats(float deltaTime) {
            if (frameTimeHistory.size() >= MAX_SAMPLES) {
                frameTimeHistory.erase(frameTimeHistory.begin());
            }
            frameTimeHistory.push_back(deltaTime);

            // Sort frame times (higher times = lower FPS)
            std::vector<float> sortedTimes = frameTimeHistory;
            std::sort(sortedTimes.begin(), sortedTimes.end(), std::greater<float>());

            // Calculate percentile indices
            size_t onePercentIndex = frameTimeHistory.size() / 10;
            size_t pointOnePercentIndex = frameTimeHistory.size() / 100;

            // Calculate 1% and 0.1% lows
            onePercentLow = onePercentIndex > 0 ?
                1.0f / sortedTimes[onePercentIndex - 1] : 0.0f;
            pointOnePercentLow = pointOnePercentIndex > 0 ?
                1.0f / sortedTimes[pointOnePercentIndex - 1] : 0.0f;
        }
    };

    ImGuiPerformanceLayer::ImGuiPerformanceLayer(const char* name, int32_t priority)
        : ImGuiLayer(name, priority)
        , mImpl(new Impl)
    {
    }

    ImGuiPerformanceLayer::~ImGuiPerformanceLayer() {
        delete mImpl;
    }

    void ImGuiPerformanceLayer::onUpdate(float deltaTime) {
        mImpl->updateStats(deltaTime);
        ImGuiLayer::onUpdate(deltaTime);

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowBgAlpha(0.35f);

        ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
        ImGui::Dummy(ImVec2(130, 1));
        ImGui::PopStyleVar();

        mImpl->lastWindowPos = ImGui::GetWindowPos();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        windowPos.x = std::clamp(windowPos.x, main_viewport->WorkPos.x,
            main_viewport->WorkPos.x + main_viewport->WorkSize.x - windowSize.x);
        windowPos.y = std::clamp(windowPos.y, main_viewport->WorkPos.y,
            main_viewport->WorkPos.y + main_viewport->WorkSize.y - windowSize.y);
        ImGui::SetWindowPos(windowPos);

        // Display FPS stats
        ImGui::Text("Average FPS: %6.1f", ImGui::GetIO().Framerate);
        ImGui::Text("1%% Low: %6.1f", mImpl->onePercentLow);
        ImGui::Text("0.1%% Low: %6.1f", mImpl->pointOnePercentLow);
        ImGui::End();
    }
}