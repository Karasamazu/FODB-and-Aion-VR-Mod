#include "dump_callstack.h"
#include "util/log.h"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace vrdebug
{
    void dump_callstack(int maxFrames)
    {
        void* stack[64];
        USHORT frames = CaptureStackBackTrace(0, maxFrames, stack, nullptr);

        LOG_INFO("[VR] ---- Callstack ----");

        for (USHORT i = 0; i < frames; i++)
        {
            LOG_INFO("[VR]   %p", stack[i]);
        }

        LOG_INFO("[VR] -------------------");
    }
}

#else

namespace vrdebug
{
    void dump_callstack(int) {}
}

#endif
