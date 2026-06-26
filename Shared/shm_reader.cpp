#include <windows.h>
#include <cstdio>
#include "shm_structs.h"

int main() {
    const char* shmName = "aion_vr_state"; // Name muss zum Linux-Teil passen

    HANDLE hMap = OpenFileMappingA(
        FILE_MAP_READ,
        FALSE,
        shmName
    );

    if (!hMap) {
        printf("OpenFileMappingA failed, GetLastError=%lu\n", GetLastError());
        return 1;
    }

    void* ptr = MapViewOfFile(
        hMap,
        FILE_MAP_READ,
        0, 0,
        sizeof(VrSharedState)
    );

    if (!ptr) {
        printf("MapViewOfFile failed, GetLastError=%lu\n", GetLastError());
        CloseHandle(hMap);
        return 1;
    }

    VrSharedState* state = reinterpret_cast<VrSharedState*>(ptr);

    std::uint64_t lastSeq = 0;

    while (true) {
        if (state->seq != lastSeq) {
            lastSeq = state->seq;
            printf("seq=%llu\n", (unsigned long long)state->seq);
            printf("view_left.m[0]=%f\n", state->view_left.m[0]);
        }
        Sleep(10);
    }

    UnmapViewOfFile(ptr);
    CloseHandle(hMap);
    return 0;
}
