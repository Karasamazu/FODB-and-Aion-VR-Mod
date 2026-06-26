#pragma once
#include <cstdint>

struct Mat4 {
    float m[16];
};

struct VrSharedState {
    std::uint64_t seq;
    Mat4 view_left;
    Mat4 view_right;
    Mat4 proj_left;
    Mat4 proj_right;
};
