#pragma once
#include "saneengine/ecs/system.hpp"
#include "saneengine/utils/api.hpp"

namespace sane::ecs {
    class SANEENGINE_API RenderSystem : public System {
    public:
        RenderSystem();
        ~RenderSystem() override;

        void onAttach(entt::registry& registry) override;
        void onDetach(entt::registry& registry) override;
        void onUpdate(entt::registry& registry, float deltaTime) override;

    private:
        uint32_t mVao{ 0 }, mVbo{ 0 };
    };
}