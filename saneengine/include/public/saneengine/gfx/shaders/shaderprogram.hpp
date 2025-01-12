#pragma once

// STL headers
#include <initializer_list>
#include <string>
#include <utility>

// External headers
#include <glm/glm.hpp>

// Public headers
#include <saneengine/utils/api.hpp>
#include <saneengine/utils/notcopyable.hpp>
#include <saneengine/gfx/shaders/shadertype.hpp>

namespace sane::gfx {
    using ShaderData = std::pair<ShaderType, std::string>;

    class SANEENGINE_API ShaderProgram : utils::NotCopyable {
    public:
        ShaderProgram(std::initializer_list<ShaderData> inShaderData, const std::string& inName = "");
        ShaderProgram(ShaderProgram&& other) noexcept;
        virtual ~ShaderProgram();

        uint32_t getProgramId() const;

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