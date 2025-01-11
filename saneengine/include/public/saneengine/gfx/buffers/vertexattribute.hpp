#pragma once

#include <cstdint>
#include <map>

#define ATTRIB_OFFSET_INFO(TYPE, MEMBER) sizeof(TYPE), [] {TYPE ; return (uint64_t)& .MEMBER - (uint64_t)& ; }()

namespace sane::gfx
{
    struct VertexAttribute
    {
        int32_t position;
        int32_t count;
        uint32_t type;
        bool normalized;
        int32_t stride;
        uint64_t offset;
        int32_t instances;
    };

    class VertexAttribInfo
    {
    public:
        explicit VertexAttribInfo(std::initializer_list<VertexAttribute> inAttribs)
        {
            for (const auto& attrib : inAttribs)
            {
                if (attrib.position == -1)
                {
                    continue;
                }
                attribs[(uint32_t)attrib.position] = attrib;
            }
        }

        std::map<uint32_t, VertexAttribute> attribs;
    };

} // namespace sane::gfx
