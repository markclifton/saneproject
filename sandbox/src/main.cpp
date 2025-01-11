#include <saneengine/entrypoint.hpp>
#include <saneengine/layer/layer.hpp>
#include <saneengine/layer/imguilayer.hpp>

class GameLayer : public sane::Layer {
public:
    GameLayer(sane::Application* inApplication) : Layer("Game"), mApplication(inApplication) {}

    void onUpdate(float deltaTime) override {
        // Game logic
        static bool sOnce = [this] {
            mApplication->pushLayer(std::make_unique<sane::ImGuiLayer>());
            return true;
            }();

    }

    void onRender() override {
        // Render game objects
    }

    sane::Application* mApplication;
};

class SandboxApplication : public sane::Application {
public:
    SandboxApplication() : sane::Application("Sandbox", 1280, 720) {
        pushLayer(std::make_unique<GameLayer>(this));
    }
};

sane::Application* createApplication() {
    return new SandboxApplication();
}
