#include "saneengine/layer/imguilayer.hpp"
#include "saneengine/layer/imguicontextmanager.hpp"
#include <imgui.h>
#include <GLFW/glfw3.h>

namespace sane {
    class ImGuiLayer::Impl {
    public:
        ImGuiContextManager contextManager;

        void initializeContext() {
            if (!contextManager.isInitialized()) {
                contextManager.initialize();
            }
        }

        ~Impl() = default;
    };

    ImGuiLayer::ImGuiLayer(const char* name, int32_t priority)
        : Layer(name, priority)
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
    }

    void ImGuiLayer::onRender() {
        if (!mImpl->contextManager.isInitialized()) return;
        mImpl->contextManager.endFrame();
    }
}