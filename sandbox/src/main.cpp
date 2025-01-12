#include <saneengine/entrypoint.hpp>
#include <saneengine/layer/layer.hpp>
#include <saneengine/layer/imguilayer.hpp>
#include <saneengine/ecs/systems/rendersystem.hpp>
#include <saneengine/utils/uuid.hpp>

class SandboxApplication : public sane::Application {
public:
    SandboxApplication() : sane::Application("Sandbox", 1280, 720) {
        // Add layers
        pushLayer(std::make_unique<sane::ImGuiLayer>());

        // Add systems
        // mRenderSystemId = startSystem(std::make_unique<sane::ecs::RenderSystem>());
    }

    ~SandboxApplication()
    {
        // stopSystem(mRenderSystemId);
    }

private:
    sane::utils::UUID mRenderSystemId;
};

std::unique_ptr<sane::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
