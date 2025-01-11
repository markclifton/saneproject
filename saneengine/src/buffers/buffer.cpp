#include "saneengine/gfx/buffers/buffer.hpp"

#include <stdexcept>

namespace sane::gfx {

    Buffer::Buffer(GLuint inTarget, uint32_t inSize, GLenum inUsage, const void* inData)
        : mTargetType(inTarget)
        , mBuffer(0)
        , mSize(inSize)
    {
        glGenBuffers(1, &mBuffer);
        if (!mBuffer) {
            throw std::runtime_error("Failed to create buffer");
        }

        glBindBuffer(mTargetType, mBuffer);
        glBufferData(mTargetType, mSize, inData, inUsage);
        glBindBuffer(mTargetType, 0);
    }

    Buffer::~Buffer() {
        if (mBuffer) {
            glDeleteBuffers(1, &mBuffer);
            mBuffer = 0;
        }
    }

    void Buffer::bind() const {
        glBindBuffer(mTargetType, mBuffer);
    }

    void Buffer::unbind() const {
        glBindBuffer(mTargetType, 0);
    }

    void Buffer::update(uint32_t inSize, uint32_t inOffset, const void* inData) {
        if (inOffset + inSize > mSize) {
            throw std::runtime_error("Buffer update out of bounds");
        }

        glBindBuffer(mTargetType, mBuffer);
        glBufferSubData(mTargetType, inOffset, inSize, inData);
        glBindBuffer(mTargetType, 0);
    }

    void Buffer::move(uint32_t inSize, uint32_t inSrcOffset, uint32_t inDstOffset) {
        if (inSrcOffset + inSize > mSize || inDstOffset + inSize > mSize) {
            throw std::runtime_error("Buffer move out of bounds");
        }

        glBindBuffer(GL_COPY_READ_BUFFER, mBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, mBuffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
            inSrcOffset, inDstOffset, inSize);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

    void Buffer::clone(Buffer& inBuffer) {
        if (mSize != inBuffer.mSize) {
            throw std::runtime_error("Buffer sizes do not match");
        }

        glBindBuffer(GL_COPY_READ_BUFFER, mBuffer);
        glBindBuffer(GL_COPY_WRITE_BUFFER, inBuffer.mBuffer);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER,
            0, 0, mSize);
        glBindBuffer(GL_COPY_READ_BUFFER, 0);
        glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
    }

} // namespace sane::gfx