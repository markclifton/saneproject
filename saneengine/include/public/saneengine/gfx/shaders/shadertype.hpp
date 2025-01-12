#pragma once
#include "saneengine/utils/api.hpp"
#include <glad/glad.h>

namespace sane::gfx {
    enum class ShaderType {
        Vertex = GL_VERTEX_SHADER
        , Fragment = GL_FRAGMENT_SHADER
        , Geometry = GL_GEOMETRY_SHADER
        // ,Compute = GL_COMPUTE_SHADER
    };

    inline GLenum toGLenum(ShaderType type) {
        return static_cast<GLenum>(type);
    }
}