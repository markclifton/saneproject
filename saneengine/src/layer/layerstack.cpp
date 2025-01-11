#include "saneengine/layer/layerstack.hpp"
#include "saneengine/layer/layer.hpp"
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace sane {
    class LayerStack::Impl {
    public:
        std::vector<std::unique_ptr<Layer>> layers;
    };

    LayerStack::LayerStack() : mImpl(new Impl()) {}
    LayerStack::~LayerStack() {
        delete mImpl;
    }

    void LayerStack::pushLayer(std::unique_ptr<Layer> layer) {
        if (!layer) {
            throw std::runtime_error("Cannot push null layer");
        }

        layer->onAttach();

        auto it = std::lower_bound(mImpl->layers.begin(), mImpl->layers.end(), layer,
            [](const auto& a, const auto& b) {
                return a->getPriority() < b->getPriority();
            });

        mImpl->layers.insert(it, std::move(layer));
    }

    void LayerStack::popLayer() {
        if (mImpl->layers.empty()) {
            throw std::runtime_error("Cannot pop from empty layer stack");
        }
        mImpl->layers.back()->onDetach();
        mImpl->layers.pop_back();
    }

    const std::vector<std::unique_ptr<Layer>>& LayerStack::getLayers() const {
        return mImpl->layers;
    }

    std::vector<std::unique_ptr<Layer>>& LayerStack::getLayers() {
        return mImpl->layers;
    }
}