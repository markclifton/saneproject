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
            if (vertex.vao) {
                glDeleteVertexArrays(1, &vertex.vao);
            }
            if (vertex.vbo) {
                glDeleteBuffers(1, &vertex.vbo);
            }
        }
    }

    void RenderSystem::onUpdate(entt::registry& registry, float deltaTime) {
        if (!isEnabled()) return;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto view = registry.view<ShaderComponent, VertexComponent, TransformComponent>();
        for (auto entity : view) {
            const auto& shader = view.get<ShaderComponent>(entity);
            const auto& vertex = view.get<VertexComponent>(entity);
            const auto& transform = view.get<TransformComponent>(entity);

            if (!shader.initialized || !vertex.initialized) continue;

            glUseProgram(shader.programId);
            glBindVertexArray(vertex.vao);

            // Set transform uniforms
            glm::mat4 model = glm::translate(glm::mat4(1.0f), transform.position);
            model = glm::rotate(model, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, transform.scale);

            GLint modelLoc = glGetUniformLocation(shader.programId, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex.vertices.size() / 3);
        }
    }
}