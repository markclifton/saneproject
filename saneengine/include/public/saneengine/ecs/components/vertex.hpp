#pragma once
#include <vector>
#include <cstdint>

namespace sane::ecs {
    struct VertexComponent {
        uint32_t vao{ 0 };
        uint32_t vbo{ 0 };
        std::vector<float> vertices;
        bool initialized{ false };
    };
}