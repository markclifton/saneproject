#include "saneengine/utils/uuid.hpp"
#include <random>

namespace sane::utils {
    UUID generateUUID() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<UUID> dis;
        return dis(gen);
    }
}