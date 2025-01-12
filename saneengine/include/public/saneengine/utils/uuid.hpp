#pragma once
#include <cstdint>

namespace sane::utils {
    using UUID = uint64_t;
    UUID generateUUID();
}