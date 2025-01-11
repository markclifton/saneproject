#pragma once

namespace sane::utils {
    class NotCopyable {
    protected:
        NotCopyable() = default;
        ~NotCopyable() = default;

        NotCopyable(const NotCopyable&) = delete;
        NotCopyable& operator=(const NotCopyable&) = delete;
    };
} // namespace sane::utils