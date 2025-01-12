#pragma once

#include "saneengine/utils/api.hpp"
#include "saneengine/utils/notcopyable.hpp"
#include "saneengine/utils/uuid.hpp"
#include <entt/entt.hpp>
#include <memory>

namespace sane::ecs {
    class System;

    class SANEENGINE_API SystemManager : public utils::NotCopyable {
    public:
        SystemManager();
        ~SystemManager();

        void startup();
        void shutdown();

        void addSystem(std::unique_ptr<System> system);
        void removeSystem(utils::UUID systemId);
        System* getSystem(utils::UUID systemId);

        void update(float deltaTime);
        entt::registry& getRegistry();

    private:
        class Impl;
        Impl* mImpl;
    };
}