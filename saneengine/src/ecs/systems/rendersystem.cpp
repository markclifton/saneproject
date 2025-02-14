#include "saneengine/ecs/systems/rendersystem.hpp"
#include "saneengine/ecs/components/shader.hpp"
#include "saneengine/ecs/components/vertex.hpp"
#include "saneengine/ecs/components/transform.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

        glDeleteVertexArrays(1, &mVao);
        glDeleteBuffers(1, &mVbo);
    }

    void RenderSystem::onUpdate(entt::registry& registry, float deltaTime) {
        if (!isEnabled()) return;

        static bool sOnce = [this] {
            glGenVertexArrays(1, &mVao);
            glGenBuffers(1, &mVbo);

            return true;
            }();

        glBindVertexArray(mVao);

        auto view = registry.view<ShaderComponent, VertexComponent, TransformComponent>();
        for (auto entity : view) {
            const auto& shader = view.get<ShaderComponent>(entity);
            auto& vertex = view.get<VertexComponent>(entity);
            const auto& transform = view.get<TransformComponent>(entity);

            glBindBuffer(GL_ARRAY_BUFFER, mVbo);
            glBufferData(GL_ARRAY_BUFFER,
                vertex.getVertexCount() * 3 * sizeof(float),
                vertex.getVertices(),
                GL_STATIC_DRAW);


            // Setup attributes
            for (const auto& attr : vertex.getAttributes()) {
                glEnableVertexAttribArray(attr.position);
                glVertexAttribPointer(
                    attr.position,
                    attr.count,
                    attr.type,
                    attr.normalized ? GL_TRUE : GL_FALSE,
                    attr.stride,
                    (void*)attr.offset
                );
            }

            glUseProgram(shader.programId);

            // Set transform uniforms
            glm::mat4 model = glm::translate(glm::mat4(1.0f), { transform.getPositionX(), transform.getPositionY(), transform.getPositionZ() });
            model = glm::rotate(model, transform.getRotationX(), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, transform.getRotationY(), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, transform.getRotationZ(), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, { transform.getScaleX(), transform.getScaleY(), transform.getScaleZ() });

            GLint modelLoc = glGetUniformLocation(shader.programId, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex.getVertexCount());
        }

        glBindVertexArray(0);
    }

}