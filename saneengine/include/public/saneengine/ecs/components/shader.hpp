#pragma once
#include <cstdint>

namespace sane::ecs {
    struct ShaderComponent {
        uint32_t programId{ 0 };
        bool initialized{ false };
    };
}