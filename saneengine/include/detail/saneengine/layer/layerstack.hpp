#pragma once

#include <memory>
#include <vector>

#include "saneengine/utils/notcopyable.hpp" 

namespace sane {
    class Layer;

    class LayerStack : utils::NotCopyable {
    public:
        LayerStack();
        ~LayerStack();

        void pushLayer(std::unique_ptr<Layer> layer);
        void popLayer();

        const std::vector<std::unique_ptr<Layer>>& getLayers() const;
        std::vector<std::unique_ptr<Layer>>& getLayers();

    private:
        class Impl;
        Impl* mImpl;
    };
}