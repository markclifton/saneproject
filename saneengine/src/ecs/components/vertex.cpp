#include "saneengine/ecs/components/vertex.hpp"
#include <cstring>

namespace sane::ecs {
    VertexComponent::VertexComponent() = default;

    VertexComponent::~VertexComponent() {
        delete[] vertices;
    }

    VertexComponent::VertexComponent(const VertexComponent& other) {
        if (other.vertices && other.vertexCount > 0) {
            vertices = new float[other.vertexCount];
            std::memcpy(vertices, other.vertices, other.vertexCount * sizeof(float));
            vertexCount = other.vertexCount;
        }
    }

    VertexComponent& VertexComponent::operator=(const VertexComponent& other) {
        if (this != &other) {
            delete[] vertices;
            if (other.vertices && other.vertexCount > 0) {
                vertices = new float[other.vertexCount];
                std::memcpy(vertices, other.vertices, other.vertexCount * sizeof(float));
                vertexCount = other.vertexCount;
            }
        }
        return *this;
    }

    void VertexComponent::setVertexData(const float* data, size_t count) {
        delete[] vertices;
        vertices = new float[count * 3];
        vertexCount = count;
        std::memcpy(vertices, data, 3 * count * sizeof(float));

        setInitialized(true);
    }

    void VertexComponent::setVertexData(std::initializer_list<float> data, size_t count) {
        setVertexData(data.begin(), count);
    }

    const float* VertexComponent::getVertices() const { return vertices; }
    size_t VertexComponent::getVertexCount() const { return vertexCount; }
    bool VertexComponent::isInitialized() const { return initialized; }
    void VertexComponent::setInitialized(bool value) { initialized = value; }
}