#include "saneengine/ecs/system.hpp"
#include <string>

namespace sane::ecs {
    class System::Impl {
    public:
        std::string name;
        bool enabled{ true };
        int32_t priority{ 0 };
        utils::UUID id{ 0 };
    };

    System::System(const char* name, int32_t priority) : mImpl(new Impl) {
        mImpl->name = name;
        mImpl->priority = priority;
    }

    System::~System() {
        delete mImpl;
    }

    const char* System::getName() const {
        return mImpl->name.c_str();
    }

    bool System::isEnabled() const {
        return mImpl->enabled;
    }

    void System::setEnabled(bool enabled) {
        mImpl->enabled = enabled;
    }

    int32_t System::getPriority() const {
        return mImpl->priority;
    }

    utils::UUID System::getId() const {
        return mImpl->id;
    }

    void System::setId(utils::UUID id) {
        mImpl->id = id;
    }
}