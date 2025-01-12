#include "saneengine/ecs/systemmanager.hpp"
#include "saneengine/ecs/system.hpp"
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace sane::ecs {
    class SystemManager::Impl {
    public:
        std::vector<std::unique_ptr<System>> systems;
        std::unique_ptr<entt::registry> registry{ std::make_unique<entt::registry>() };
    };

    SystemManager::SystemManager() : mImpl(new Impl) {}

    SystemManager::~SystemManager() {
        shutdown();
        delete mImpl;
    }

    void SystemManager::addSystem(std::unique_ptr<System> system) {
        if (!system) {
            throw std::runtime_error("Cannot add null system");
        }

        system->onAttach(*mImpl->registry);

        auto insertPos = std::lower_bound(mImpl->systems.begin(), mImpl->systems.end(), system,
            [](const auto& a, const auto& b) {
                return a->getPriority() < b->getPriority();
            });

        mImpl->systems.insert(insertPos, std::move(system));
    }

    void SystemManager::removeSystem(utils::UUID systemId) {
        auto it = std::find_if(mImpl->systems.begin(), mImpl->systems.end(),
            [systemId](const auto& sys) {
                return sys->getId() == systemId;
            });

        if (it != mImpl->systems.end()) {
            (*it)->onDetach(*mImpl->registry);
            mImpl->systems.erase(it);
        }
    }

    System* SystemManager::getSystem(utils::UUID systemId) {
        auto it = std::find_if(mImpl->systems.begin(), mImpl->systems.end(),
            [systemId](const auto& sys) {
                return sys->getId() == systemId;
            });

        return it != mImpl->systems.end() ? it->get() : nullptr;
    }

    void SystemManager::update(float deltaTime) {
        for (auto& system : mImpl->systems) {
            if (system->isEnabled()) {
                system->onUpdate(*mImpl->registry, deltaTime);
            }
        }
    }

    entt::registry& SystemManager::getRegistry() {
        return *mImpl->registry;
    }

    void SystemManager::startup() {
        for (auto& system : mImpl->systems) {
            system->onStartup();
        }
    }

    void SystemManager::shutdown() {
        for (auto& system : mImpl->systems) {
            system->onShutdown();
            system->onDetach(*mImpl->registry);
        }
        mImpl->systems.clear();
    }
}