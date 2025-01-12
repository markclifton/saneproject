#pragma once
#include "saneengine/utils/api.hpp"

namespace sane::ecs {
    class SANEENGINE_API TransformComponent {
    public:
        // Position
        float getPositionX() const { return posX; }
        float getPositionY() const { return posY; }
        float getPositionZ() const { return posZ; }
        void setPosition(float x, float y, float z) {
            posX = x; posY = y; posZ = z;
        }

        // Rotation
        float getRotationX() const { return rotX; }
        float getRotationY() const { return rotY; }
        float getRotationZ() const { return rotZ; }
        void setRotation(float x, float y, float z) {
            rotX = x; rotY = y; rotZ = z;
        }

        // Scale
        float getScaleX() const { return scaleX; }
        float getScaleY() const { return scaleY; }
        float getScaleZ() const { return scaleZ; }
        void setScale(float x, float y, float z) {
            scaleX = x; scaleY = y; scaleZ = z;
        }

    private:
        // Position
        float posX{ 0.0f };
        float posY{ 0.0f };
        float posZ{ 0.0f };

        // Rotation
        float rotX{ 0.0f };
        float rotY{ 0.0f };
        float rotZ{ 0.0f };

        // Scale
        float scaleX{ 1.0f };
        float scaleY{ 1.0f };
        float scaleZ{ 1.0f };
    };
}