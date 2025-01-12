#include "saneengine/ecs/systems/rendersystem.hpp"
#include "saneengine/ecs/components/shader.hpp"
#include "saneengine/ecs/components/vertex.hpp"
#include "saneengine/ecs/components/transform.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace sane::ecs {
    RenderSystem::RenderSystem()
        : System("RenderSystem")
    {
    }

    RenderSystem::~RenderSystem() = default;

    void RenderSystem::onAttach(entt::registry& registry) {
        registry.clear<ShaderComponent>();
        registry.clear<VertexComponent>();
        registry.clear<TransformComponent>();
    }

    void RenderSystem::onDetach(entt::registry& registry) {
        auto view = registry.view<ShaderComponent, VertexComponent>();
        for (auto [entity, shader, vertex] : view.each()) {
            if (shader.programId) {
                glDeleteProgram(shader.programId);
            }

        }
    }

    void RenderSystem::onUpdate(entt::registry& registry, float deltaTime) {
        if (!isEnabled()) return;

        auto view = registry.view<ShaderComponent, VertexComponent, TransformComponent>();
        for (auto entity : view) {
            const auto& shader = view.get<ShaderComponent>(entity);
            const auto& vertex = view.get<VertexComponent>(entity);
            const auto& transform = view.get<TransformComponent>(entity);

            if (!shader.initialized || !vertex.isInitialized()) continue;

            glUseProgram(shader.programId);

            // Set transform uniforms
            glm::mat4 model = glm::translate(glm::mat4(1.0f), { transform.getPositionX(), transform.getPositionY(), transform.getPositionZ() });
            model = glm::rotate(model, transform.getRotationX(), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, transform.getRotationY(), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, transform.getRotationZ(), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, { transform.getScaleX(), transform.getScaleY(), transform.getScaleZ() });

            GLint modelLoc = glGetUniformLocation(shader.programId, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex.getVertexCount());
        }
    }
}