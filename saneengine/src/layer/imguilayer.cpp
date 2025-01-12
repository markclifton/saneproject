#include "saneengine/layer/imguilayer.hpp"
#include "saneengine/layer/imguicontextmanager.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace sane {
    class ImGuiLayer::Impl {
    public:
        ImVec2 lastFpsWindowPos{ 10.0f, 10.0f };
        ImGuiContextManager contextManager;

        void initializeContext() {
            if (!contextManager.isInitialized()) {
                contextManager.initialize();
            }
        }

        ~Impl() = default;
    };

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGui Layer", -1)
        , mImpl(new Impl)
    {
    }

    ImGuiLayer::~ImGuiLayer() {
        delete mImpl;
    }

    void ImGuiLayer::onAttach() {
    }

    void ImGuiLayer::onDetach() {
    }

    void ImGuiLayer::onUpdate(float deltaTime) {
        mImpl->initializeContext();
        if (!mImpl->contextManager.isInitialized()) return;

        mImpl->contextManager.beginFrame();
        mImpl->contextManager.updateStats(deltaTime);

        // Add FPS overlay with screen edge clamping
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

        mImpl->lastFpsWindowPos = ImGui::GetWindowPos();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        windowPos.x = std::clamp(windowPos.x, main_viewport->WorkPos.x,
            main_viewport->WorkPos.x + main_viewport->WorkSize.x - windowSize.x);
        windowPos.y = std::clamp(windowPos.y, main_viewport->WorkPos.y,
            main_viewport->WorkPos.y + main_viewport->WorkSize.y - windowSize.y);
        ImGui::SetWindowPos(windowPos);

        ImGui::Text("Average FPS: %6.1f", mImpl->contextManager.getAverageFPS());
        ImGui::Text("1%% Low: %6.1f", mImpl->contextManager.getOnePercentLow());
        ImGui::Text("0.1%% Low: %6.1f", mImpl->contextManager.getPointOnePercentLow());
        ImGui::End();
    }

    void ImGuiLayer::onRender() {
        if (!mImpl->contextManager.isInitialized()) return;
        mImpl->contextManager.endFrame();
    }
}