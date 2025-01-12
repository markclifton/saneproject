#include "saneengine/layer/imguicontext.hpp"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <atomic>
#include <vector>
#include <algorithm>
#include <numeric>

namespace sane {
    class ImGuiContext::Impl {
    public:
        std::atomic<int> refCount{ 0 };
        bool initialized{ false };
        std::vector<float> frameTimeHistory;
        float averageFPS{ 0.0f };
        float onePercentLow{ 0.0f };
        float pointOnePercentLow{ 0.0f };
        bool frameStarted{ false };
    };

    ImGuiContext& ImGuiContext::get() {
        static ImGuiContext instance;
        return instance;
    }

    ImGuiContext::ImGuiContext() : mImpl(new Impl) {}

    ImGuiContext::~ImGuiContext() {
        delete mImpl;
    }

    bool ImGuiContext::isInitialized() const {
        return mImpl->initialized;
    }

    void ImGuiContext::incrementRef() {
        if (++mImpl->refCount == 1) {
            initialize();
        }
    }

    void ImGuiContext::decrementRef() {
        if (--mImpl->refCount == 0) {
            shutdown();
        }
    }

    float ImGuiContext::getAverageFPS() const {
        return mImpl->averageFPS;
    }

    float ImGuiContext::getOnePercentLow() const {
        return mImpl->onePercentLow;
    }

    float ImGuiContext::getPointOnePercentLow() const {
        return mImpl->pointOnePercentLow;
    }

    void ImGuiContext::initialize() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_DockingEmptyBg] = { 0.1f, 0.1f, 0.1f, 0.0f };

        ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
        ImGui_ImplOpenGL3_Init("#version 330");

        mImpl->initialized = true;
    }

    void ImGuiContext::shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        mImpl->initialized = false;
    }

    void ImGuiContext::updateStats(float deltaTime) {
        static constexpr size_t MAX_FRAMES = 100;

        if (mImpl->frameTimeHistory.size() < MAX_FRAMES) {
            mImpl->frameTimeHistory.push_back(deltaTime);
        }
        else {
            static size_t currentIndex = 0;
            mImpl->frameTimeHistory[currentIndex] = deltaTime;
            currentIndex = (currentIndex + 1) % MAX_FRAMES;
        }

        float avgFrameTime = std::accumulate(mImpl->frameTimeHistory.begin(),
            mImpl->frameTimeHistory.end(), 0.0f) / mImpl->frameTimeHistory.size();
        mImpl->averageFPS = 1.0f / avgFrameTime;

        std::vector<float> sortedTimes = mImpl->frameTimeHistory;
        std::sort(sortedTimes.begin(), sortedTimes.end(), std::greater<float>());

        size_t onePercentIndex = mImpl->frameTimeHistory.size() / 10;
        size_t pointOnePercentIndex = mImpl->frameTimeHistory.size() / 100;

        mImpl->onePercentLow = onePercentIndex > 0 ? 1.0f / sortedTimes[onePercentIndex - 1] : 0.0f;
        mImpl->pointOnePercentLow = pointOnePercentIndex > 0 ? 1.0f / sortedTimes[pointOnePercentIndex - 1] : 0.0f;
    }

    void ImGuiContext::beginFrame(float deltaTime) {
        if (!mImpl->frameStarted) {
            updateStats(deltaTime);

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

            mImpl->frameStarted = true;
        }
    }

    void ImGuiContext::endFrame() {
        if (mImpl->frameStarted) {
            ImGui::End();

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
}