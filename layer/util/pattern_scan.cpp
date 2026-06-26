#include "pattern_scan.h"

namespace pattern
{
    const uint8_t* find_pattern(const uint8_t* base, size_t size,
                                const char* pattern, const char* mask)
    {
        size_t patternLen = 0;
        while (mask[patternLen]) ++patternLen;

        for (size_t i = 0; i <= size - patternLen; ++i)
        {
            bool found = true;
            for (size_t j = 0; j < patternLen; ++j)
            {
                if (mask[j] == 'x' && base[i + j] != (uint8_t)pattern[j])
                {
                    found = false;
                    break;
                }
            }
            if (found)
                return base + i;
        }
        return nullptr;
    }
}
