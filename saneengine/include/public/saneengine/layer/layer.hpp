#pragma once

#include <cstdint>

#include "saneengine/utils/api.hpp"
#include "saneengine/utils/notcopyable.hpp"

namespace sane {
    class SANEENGINE_API Layer : public utils::NotCopyable {
    public:
        Layer(const char* name = "Layer", int32_t priority = 0);
        virtual ~Layer();

        virtual void onAttach() {}
        virtual void onDetach() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}

        const char* getName() const;
        int32_t getPriority() const;
        void setPriority(int32_t priority);

    private:
        class Impl;
        Impl* mImpl;
    };
}