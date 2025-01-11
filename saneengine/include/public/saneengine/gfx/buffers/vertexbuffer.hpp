#pragma once

#include <cstdint>

#include "saneengine/gfx/buffers/buffer.hpp"

namespace sane::gfx
{
    class VertexBuffer : public Buffer
    {
    public:
        VertexBuffer(uint32_t inSize, GLenum inUsage = GL_DYNAMIC_DRAW, const void* inData = nullptr);
        virtual ~VertexBuffer() = default;
    };
} // namespace sane::gfx