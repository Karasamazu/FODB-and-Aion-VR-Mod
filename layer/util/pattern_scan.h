#pragma once
#include <cstdint>
#include <cstddef>

namespace pattern
{
    const uint8_t* find_pattern(const uint8_t* base, size_t size,
                                const char* pattern, const char* mask);
}
