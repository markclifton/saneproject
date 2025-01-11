#include "saneengine/layer/imguilayer.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace sane {
    class ImGuiLayer::Impl {
    public:
        bool initialized{ false };
        ImVec2 lastFpsWindowPos{ 10.0f, 10.0f };
        std::vector<float> frameTimeHistory;
        size_t currentFrameIndex{ 0 };
        static constexpr size_t MAX_FRAMES = 100;

        float averageFPS{ 0.0f };
        float onePercentLow{ 0.0f };
        float pointOnePercentLow{ 0.0f };

        void updateStats(float deltaTime) {
            if (frameTimeHistory.size() < MAX_FRAMES) {
                frameTimeHistory.push_back(deltaTime);
            }
            else {
                frameTimeHistory[currentFrameIndex] = deltaTime;
                currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAMES;
            }

            // Calculate average FPS
            float avgFrameTime = std::accumulate(frameTimeHistory.begin(),
                frameTimeHistory.end(), 0.0f) / frameTimeHistory.size();
            averageFPS = 1.0f / avgFrameTime;

            // Calculate percentile lows
            std::vector<float> sortedTimes = frameTimeHistory;
            std::sort(sortedTimes.begin(), sortedTimes.end(), std::greater<float>());

            size_t onePercentIndex = frameTimeHistory.size() / 10;
            size_t pointOnePercentIndex = frameTimeHistory.size() / 100;

            if (onePercentIndex > 0) {
                onePercentLow = 1.0f / sortedTimes[onePercentIndex - 1];
            }

            if (pointOnePercentIndex > 0) {
                pointOnePercentLow = 1.0f / sortedTimes[pointOnePercentIndex - 1];
            }
        }
    };

    ImGuiLayer::ImGuiLayer()
        : Layer("ImGui Layer", -1)  // High priority to render last
        , mImpl(new Impl)
    {
    }

    ImGuiLayer::~ImGuiLayer() {
        delete mImpl;
    }

    void ImGuiLayer::onAttach() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;

        const ImVec4 bgColor = ImVec4(0.1f, 0.1f, 0.1f, 0.0f);
        style.Colors[ImGuiCol_DockingEmptyBg] = bgColor;

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
        ImGui_ImplOpenGL3_Init("#version 450");

        mImpl->initialized = true;
    }

    void ImGuiLayer::onDetach() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::onUpdate(float deltaTime) {
        if (!mImpl->initialized) return;

        mImpl->updateStats(deltaTime);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Setup dockspace
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoBackground;  // Add transparent background
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("DockSpace", nullptr, window_flags);
        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        // Add FPS overlay with screen edge clamping
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowBgAlpha(0.35f);

        // Set initial position only once
        ImGui::SetNextWindowPos(mImpl->lastFpsWindowPos, ImGuiCond_FirstUseEver);

        ImGui::Begin("FPS", nullptr, ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav);

        // Force minimum window width
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 4));
        ImGui::Dummy(ImVec2(130, 1));
        ImGui::PopStyleVar();

        // Store position for next frame
        mImpl->lastFpsWindowPos = ImGui::GetWindowPos();

        // Clamp to viewport
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        windowPos.x = std::clamp(windowPos.x, main_viewport->WorkPos.x,
            main_viewport->WorkPos.x + main_viewport->WorkSize.x - windowSize.x);
        windowPos.y = std::clamp(windowPos.y, main_viewport->WorkPos.y,
            main_viewport->WorkPos.y + main_viewport->WorkSize.y - windowSize.y);
        ImGui::SetWindowPos(windowPos);

        ImGui::Text("Average FPS: %6.1f", mImpl->averageFPS);
        ImGui::Text("1%% Low: %6.1f", mImpl->onePercentLow);
        ImGui::Text("0.1%% Low: %6.1f", mImpl->pointOnePercentLow);
        ImGui::End();

        ImGui::End();
    }

    void ImGuiLayer::onRender() {
        if (!mImpl->initialized) return;

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
}