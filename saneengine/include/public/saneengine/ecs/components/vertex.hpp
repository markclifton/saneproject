#pragma once
#include "saneengine/utils/api.hpp"
#include <cstdint>
#include <initializer_list>

namespace sane::ecs {
    class SANEENGINE_API VertexComponent {
    public:
        VertexComponent();
        ~VertexComponent();
        VertexComponent(const VertexComponent& other);
        VertexComponent& operator=(const VertexComponent& other);

        void setVertexData(const float* data, size_t count);
        void setVertexData(std::initializer_list<float> data);

        const float* getVertices() const;
        size_t getVertexCount() const;

        bool isInitialized() const;
        void setInitialized(bool value);

    private:
        float* vertices{ nullptr };
        size_t vertexCount{ 0 };
        bool initialized{ false };
    };
}