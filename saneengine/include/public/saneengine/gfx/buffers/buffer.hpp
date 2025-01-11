#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <saneengine/utils/notcopyable.hpp>

namespace sane::gfx
{
    class Buffer : utils::NotCopyable
    {
    public:
        Buffer(GLuint inTarget, uint32_t inSize, GLenum inUsage = GL_DYNAMIC_DRAW, const void* inData = nullptr);
        virtual ~Buffer();

        virtual void bind() const;
        virtual void unbind() const;

        virtual void update(uint32_t inSize, uint32_t inOffset, const void* inData);
        virtual void move(uint32_t inSize, uint32_t inSrcOffset, uint32_t inDstOffset);

        virtual void clone(Buffer& inBuffer);

    protected:
        GLuint mTargetType;
        GLuint mBuffer;
        uint32_t mSize;
    };
} // namespace sane::gfx