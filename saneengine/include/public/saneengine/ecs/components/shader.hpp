#pragma once

#include "saneengine/utils/api.hpp"
#include <cstdint>

namespace sane::ecs {
    struct SANEENGINE_API ShaderComponent {
        uint32_t programId{ 0 };
        bool initialized{ false };
    };
}