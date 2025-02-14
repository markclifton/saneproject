#include <memory>

#include <saneengine/entrypoint.hpp>
#include <saneengine/ecs/systemmanager.hpp>
#include <saneengine/layer/imguiperformancelayer.hpp>
#include <saneengine/ecs/systems/rendersystem.hpp>
#include <saneengine/ecs/systemmanager.hpp>
#include <saneengine/gfx/shaders/shaderprogram.hpp>
#include <saneengine/ecs/components/vertex.hpp>
#include <saneengine/ecs/components/shader.hpp>
#include <saneengine/ecs/components/transform.hpp>

class TestLayer : public sane::Layer {
public:
    TestLayer(sane::ecs::SystemManager& inSystemManager) : Layer("TestLayer"), mSystemManager(inSystemManager)
    {
    }

    void onAttach() override {
        // Create triangle entity
        auto& registry = mSystemManager.getRegistry();
        auto entity = registry.create();

        static auto shaderProgram = std::make_unique<sane::gfx::ShaderProgram>(
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

        // Add vertex component
        auto& vertexComp = registry.emplace<sane::ecs::VertexComponent>(entity);
        vertexComp.setVertexData({
            -0.5f, -0.2887f, 0.0f,
             0.5f, -0.2887f, 0.0f,
             0.0f,  0.5774f, 0.0f
            }, 3);

        std::vector<sane::gfx::VertexAttribute> attributes;
        attributes.push_back({ 0, 3, GL_FLOAT, false, 3 * sizeof(float), 0, 0 }); // position attribute
        vertexComp.setAttributes(attributes);

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

    void onUpdate(float deltaTime) override {
        auto& registry = mSystemManager.getRegistry();
        auto view = registry.view<sane::ecs::ShaderComponent, sane::ecs::TransformComponent>();

        for (auto entity : view) {
            auto& transform = view.get<sane::ecs::TransformComponent>(entity);
            auto& shader = view.get<sane::ecs::ShaderComponent>(entity);

            // Rotate
            transform.setRotation(
                transform.getRotationX(),
                transform.getRotationY(),
                transform.getRotationZ() + 0.01f
            );
        }
    }

private:
    sane::ecs::SystemManager& mSystemManager;
};

class SandboxApplication : public sane::Application {
public:
    SandboxApplication() : sane::Application("Sandbox", 1280, 720) {
        pushLayer(std::make_unique<TestLayer>(getSystemManager()));
        pushLayer(std::make_unique<sane::ImGuiPerformanceLayer>());

        // Add systems
        mRenderSystemId = startSystem(std::make_unique<sane::ecs::RenderSystem>());
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
