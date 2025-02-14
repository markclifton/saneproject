#pragma once
#include "saneengine/utils/api.hpp"
#include "saneengine/gfx/buffers/vertexattribute.hpp"
#include <cstdint>
#include <initializer_list>
#include <vector>

namespace sane::ecs {
    class SANEENGINE_API VertexComponent {
    public:
        VertexComponent();
        ~VertexComponent();
        VertexComponent(const VertexComponent& other);
        VertexComponent& operator=(const VertexComponent& other);

        void setVertexData(const float* data, size_t count);
        void setVertexData(std::initializer_list<float> data, size_t count);

        const float* getVertices() const;
        size_t getVertexCount() const;

        bool isInitialized() const;
        void setInitialized(bool value);

        void setAttributes(const std::vector<gfx::VertexAttribute>& attrs) {
            attributes = attrs;
        }

        const std::vector<gfx::VertexAttribute>& getAttributes() const {
            return attributes;
        }

    private:
        float* vertices{ nullptr };
        size_t vertexCount{ 0 };
        bool initialized{ false };
        std::vector<gfx::VertexAttribute> attributes;
    };
}