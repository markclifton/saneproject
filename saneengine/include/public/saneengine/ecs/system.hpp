#pragma once

#include "saneengine/utils/api.hpp"
#include "saneengine/utils/notcopyable.hpp"
#include "saneengine/utils/uuid.hpp"
#include <entt/entt.hpp>

namespace sane::ecs {
    class SANEENGINE_API System : public utils::NotCopyable {
    public:
        explicit System(const char* name = "System", int32_t priority = 0);
        virtual ~System();

        virtual void onStartup() {}
        virtual void onShutdown() {}
        virtual void onAttach(entt::registry& registry) {}
        virtual void onDetach(entt::registry& registry) {}
        virtual void onUpdate(entt::registry& registry, float deltaTime) {}

        const char* getName() const;
        int32_t getPriority() const;
        bool isEnabled() const;
        void setEnabled(bool enabled);

        utils::UUID getId() const;
        void setId(utils::UUID id);

    private:
        class Impl;
        Impl* mImpl;
    };
}