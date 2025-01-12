#include "saneengine/layer/imguicontextmanager.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <algorithm>
#include <numeric>

namespace sane {
    class ImGuiContextManager::Impl {
    public:
        bool initialized{ false };
        bool frameStarted{ false };
        std::vector<float> frameTimeHistory;
        float averageFPS{ 0.0f };
        float onePercentLow{ 0.0f };
        float pointOnePercentLow{ 0.0f };
    };

    ImGuiContextManager::ImGuiContextManager() : mImpl(new Impl) {}

    ImGuiContextManager::~ImGuiContextManager() {
        shutdown();
        delete mImpl;
    }

    void ImGuiContextManager::initialize() {
        if (!mImpl->initialized) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            ImGui::StyleColorsDark();
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;

            ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
            ImGui_ImplOpenGL3_Init("#version 330");

            mImpl->initialized = true;
        }
    }

    void ImGuiContextManager::shutdown() {
        if (mImpl->initialized) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            mImpl->initialized = false;
        }
    }

    bool ImGuiContextManager::isInitialized() const {
        return mImpl->initialized;
    }

    void ImGuiContextManager::beginFrame() {
        if (!mImpl->frameStarted && mImpl->initialized) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            mImpl->frameStarted = true;
        }
    }

    void ImGuiContextManager::endFrame() {
        if (mImpl->frameStarted && mImpl->initialized) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
            mImpl->frameStarted = false;
        }
    }

    void ImGuiContextManager::updateStats(float deltaTime) {
        static constexpr size_t MAX_FRAMES = 1000;
        auto& history = mImpl->frameTimeHistory;

        if (history.size() < MAX_FRAMES) {
            history.push_back(deltaTime);
        }
        else {
            static size_t currentIndex = 0;
            history[currentIndex] = deltaTime;
            currentIndex = (currentIndex + 1) % MAX_FRAMES;
        }

        float avgFrameTime = std::accumulate(history.begin(),
            history.end(), 0.0f) / history.size();
        mImpl->averageFPS = 1.0f / avgFrameTime;

        std::vector<float> sortedTimes = history;
        std::sort(sortedTimes.begin(), sortedTimes.end(), std::greater<float>());

        size_t onePercentIndex = history.size() / 100;
        size_t pointOnePercentIndex = history.size() / 1000;

        mImpl->onePercentLow = onePercentIndex > 0 ?
            1.0f / sortedTimes[onePercentIndex - 1] : 0.0f;
        mImpl->pointOnePercentLow = pointOnePercentIndex > 0 ?
            1.0f / sortedTimes[pointOnePercentIndex - 1] : 0.0f;
    }

    float ImGuiContextManager::getAverageFPS() const { return mImpl->averageFPS; }
    float ImGuiContextManager::getOnePercentLow() const { return mImpl->onePercentLow; }
    float ImGuiContextManager::getPointOnePercentLow() const { return mImpl->pointOnePercentLow; }
}