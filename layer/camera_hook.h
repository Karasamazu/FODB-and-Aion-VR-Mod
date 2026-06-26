#pragma once
#include <cstdint>



namespace camerahook
{
    bool init(uintptr_t moduleBase);
    void apply_vr_view(float* viewMatrix); // 4x4 float matrix
}
