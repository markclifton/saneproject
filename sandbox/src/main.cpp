#include <memory>

#include <saneengine/entrypoint.hpp>
#include <saneengine/layer/imguiperformancelayer.hpp>
#include <saneengine/ecs/systems/rendersystem.hpp>
#include <saneengine/ecs/systemmanager.hpp>
#include <saneengine/gfx/shaders/shaderprogram.hpp>
#include <saneengine/ecs/components/vertex.hpp>
#include <saneengine/ecs/components/shader.hpp>
#include <saneengine/ecs/components/transform.hpp>

class SandboxApplication : public sane::Application {
public:
    SandboxApplication() : sane::Application("Sandbox", 1280, 720) {
        pushLayer(std::make_unique<sane::ImGuiPerformanceLayer>());

        // Create shader program
        auto shaderProgram = std::make_unique<sane::gfx::ShaderProgram>(
            std::initializer_list<sane::gfx::ShaderData>{
                {sane::gfx::ShaderType::Vertex, R"(
                #version 330 core
                layout (location = 0) in vec3 aPos;
                uniform mat4 model;
                void main() {
                    gl_Position = model * vec4(aPos, 1.0);
                }
                )"},
                { sane::gfx::ShaderType::Fragment, R"(
                #version 330 core
                out vec4 FragColor;
                void main() {
                    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
                }
                )" }
        },
            "TriangleShader"
        );

        // Add systems
        mRenderSystemId = startSystem(std::make_unique<sane::ecs::RenderSystem>());

        // Create triangle entity
        auto& registry = getSystemManager().getRegistry();
        auto entity = registry.create();

        // Add vertex component
        auto& vertexComp = registry.emplace<sane::ecs::VertexComponent>(entity);
        vertexComp.setVertexData({
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
            });

        // Add shader component
        auto& shaderComp = registry.emplace<sane::ecs::ShaderComponent>(entity);
        shaderComp.programId = shaderProgram->getProgramId();
        shaderComp.initialized = true;

        // Add transform component
        auto& transformComp = registry.emplace<sane::ecs::TransformComponent>(entity);
        transformComp.setPosition(0.0f, 0.0f, 0.0f);
        transformComp.setRotation(0.0f, 0.0f, 0.0f);
        transformComp.setScale(1.0f, 1.0f, 1.0f);
    }

    ~SandboxApplication() {
        stopSystem(mRenderSystemId);
    }

private:
    sane::utils::UUID mRenderSystemId;
};

std::unique_ptr<sane::Application> createApplication() {
    return std::make_unique<SandboxApplication>();
}
