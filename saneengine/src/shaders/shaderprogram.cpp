#include "saneengine/gfx/shaders/shaderprogram.hpp"

#include <glad/glad.h>
#include <stdexcept>
#include <vector>

namespace {
    uint32_t compileShader(GLenum type, const std::string& source) {
        uint32_t shader = glCreateShader(type);
        const char* sourcePtr = source.c_str();
        glShaderSource(shader, 1, &sourcePtr, nullptr);
        glCompileShader(shader);

        int success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
            glDeleteShader(shader);
            throw std::runtime_error("Shader compilation failed: " + std::string(infoLog));
        }

        return shader;
    }
}

namespace sane::gfx {
    class ShaderProgram::Impl {
    public:
        uint32_t program{ 0 };
        std::string name;
    };

    ShaderProgram::ShaderProgram(std::initializer_list<ShaderData> inShaderData, const std::string& inName)
        : mImpl(new Impl)
    {
        mImpl->program = glCreateProgram();
        mImpl->name = inName;

        std::vector<uint32_t> shaders;

        for (const auto& [type, source] : inShaderData) {
            uint32_t shader = compileShader(static_cast<GLenum>(type), source);
            glAttachShader(mImpl->program, shader);
            shaders.push_back(shader);
        }

        glLinkProgram(mImpl->program);

        int success;
        glGetProgramiv(mImpl->program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(mImpl->program, sizeof(infoLog), nullptr, infoLog);
            throw std::runtime_error("Program linking failed: " + std::string(infoLog));
        }

        for (auto shader : shaders) {
            glDeleteShader(shader);
        }
    }

    ShaderProgram::~ShaderProgram() {
        if (mImpl) {
            glDeleteProgram(mImpl->program);
            delete mImpl;
        }
    }

    uint32_t ShaderProgram::getProgramId() const {
        return mImpl->program;
    }

    void ShaderProgram::bind() const {
        glUseProgram(mImpl->program);
    }

    void ShaderProgram::unbind() const {
        glUseProgram(0);
    }

    int32_t ShaderProgram::getAttribLocation(const char* inAttrib) const {
        return glGetAttribLocation(mImpl->program, inAttrib);
    }

    int32_t ShaderProgram::getUniformLocation(const char* inUniform) const {
        return glGetUniformLocation(mImpl->program, inUniform);
    }

    void ShaderProgram::setUniform(const char* inUniform, int inValue) {
        glUniform1i(getUniformLocation(inUniform), inValue);
    }

    void ShaderProgram::setUniform(const char* inUniform, int inCount, int* inData) {
        glUniform1iv(getUniformLocation(inUniform), inCount, inData);
    }

    void ShaderProgram::setUniform(const char* inUniform, unsigned int inValue) {
        glUniform1ui(getUniformLocation(inUniform), inValue);
    }

    void ShaderProgram::setUniform(const char* inUniform, float inValue) {
        glUniform1f(getUniformLocation(inUniform), inValue);
    }

    void ShaderProgram::setUniform(const char* inUniform, const glm::vec2& inValue) {
        glUniform2fv(getUniformLocation(inUniform), 1, &inValue[0]);
    }

    void ShaderProgram::setUniform(const char* inUniform, const glm::vec3& inValue) {
        glUniform3fv(getUniformLocation(inUniform), 1, &inValue[0]);
    }

    void ShaderProgram::setUniform(const char* inUniform, const glm::vec4& inValue) {
        glUniform4fv(getUniformLocation(inUniform), 1, &inValue[0]);
    }

    void ShaderProgram::setUniform(const char* inUniform, const glm::mat4& inValue) {
        glUniformMatrix4fv(getUniformLocation(inUniform), 1, GL_FALSE, &inValue[0][0]);
    }

} // namespace sane::gfx