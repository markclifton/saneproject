#include "saneengine/layer/layer.hpp"
#include <string>

namespace sane {
    class Layer::Impl {
    public:
        std::string name;
        int32_t priority{ 0 };
    };

    Layer::Layer(const char* name, int32_t priority) : mImpl(new Impl) {
        mImpl->name = name;
        mImpl->priority = priority;
    }

    Layer::~Layer() {
        delete mImpl;
    }

    const char* Layer::getName() const {
        return mImpl->name.c_str();
    }

    int32_t Layer::getPriority() const {
        return mImpl->priority;
    }

    void Layer::setPriority(int32_t priority) {
        mImpl->priority = priority;
    }
}