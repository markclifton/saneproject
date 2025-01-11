#pragma once

// STL headers
#include <string>
#include <utility>

// External headers
#include <glm/glm.hpp>

// Public headers
#include <saneengine/api.hpp>
#include <saneengine/utils/notcopyable.hpp>

namespace sane::gfx {
    using ShaderData = std::pair<int, std::string>;

    class SANEENGINE_API ShaderProgram : utils::NotCopyable {
    public:
        explicit ShaderProgram(std::initializer_list<ShaderData> inShaderData, const std::string& inName = "");
        virtual ~ShaderProgram();

        void bind() const;
        void unbind() const;

        int32_t getAttribLocation(const char* inAttrib) const;
        int32_t getUniformLocation(const char* inUniform) const;

        void setUniform(const char* inUniform, int inValue);
        void setUniform(const char* inUniform, int inCount, int* inData);
        void setUniform(const char* inUniform, unsigned int inValue);
        void setUniform(const char* inUniform, float inValue);
        void setUniform(const char* inUniform, const glm::vec2& inValue);
        void setUniform(const char* inUniform, const glm::vec3& inValue);
        void setUniform(const char* inUniform, const glm::vec4& inValue);
        void setUniform(const char* inUniform, const glm::mat4& inValue);

    private:
        class Impl;
        Impl* mImpl;
    };
} // namespace sane::gfx