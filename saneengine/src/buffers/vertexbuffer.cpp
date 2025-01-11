#include "saneengine/gfx/buffers/vertexbuffer.hpp"

namespace sane::gfx {

    VertexBuffer::VertexBuffer(uint32_t inSize, GLenum inUsage, const void* inData)
        : Buffer(GL_ARRAY_BUFFER, inSize, inUsage, inData)
    {
    }

} // namespace sane::gfx