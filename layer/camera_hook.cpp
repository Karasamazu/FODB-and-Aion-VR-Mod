#include "camera_hook.h"
#include "xr_bridge.h"
#include "util/log.h"
#include "util/pattern_scan.h"
#include <cstring>
#include "util/module_info.h"



namespace camerahook
{
    static uintptr_t gameCameraSetMatrix = 0;

    typedef void (*SetMatrixFn)(void* camera, const float* matrix);
    static SetMatrixFn realSetMatrix = nullptr;

    bool init(uintptr_t moduleBase)
    {
        const char* sigPattern =
        "\x0F\x29\x41\x70"
        "\x0F\x29\x49\x60"
        "\x0F\x29\x41\x50"
        "\x0F\x29\x49\x40"
        "\x0F\x29\x41\x30"
        "\x0F\x29\x49\x20";

        const char* sigMask =
        "xxxxxxxxxxxxxxxxxxxxxxxx";
        size_t moduleSize = moduleinfo::get_module_size(moduleBase);
        if (!moduleSize)
        {
            LOG_ERROR("[VR] Failed to determine module size");
            return false;
        }

        const uint8_t* base = reinterpret_cast<const uint8_t*>(moduleBase);

        // Beispiel-Pattern (Platzhalter)
        //const char* pattern = "\xF3\x0F\x11\x4E\x10\xF3\x0F\x11\x46\x20";
        //const char* mask    = "xxxxxxxxxx";

        auto addr = pattern::find_pattern(base, moduleSize, sigPattern, sigMask);
        if (!addr) {
            LOG_ERROR("[VR] Camera pattern not found");
            return false;
        }

        gameCameraSetMatrix = reinterpret_cast<uintptr_t>(addr);
        realSetMatrix = reinterpret_cast<SetMatrixFn>(gameCameraSetMatrix);

        LOG_INFO("[VR] Camera hook installed at %p", (void*)gameCameraSetMatrix);


        gameCameraSetMatrix = reinterpret_cast<uintptr_t>(addr);
        realSetMatrix = reinterpret_cast<SetMatrixFn>(gameCameraSetMatrix);

        LOG_INFO("[VR] Camera hook installed at %p", (void*)gameCameraSetMatrix);
        return true;
    }


    void apply_vr_view(float* viewMatrix)
    {
        // später: Headset-Pose + Stereo-Offset
    }
}
