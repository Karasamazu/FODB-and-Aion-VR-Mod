#include "present_hook.h"
#include "xr_bridge.h"
#include "util/log.h"
#include "dump_callstack.h"

static PFN_vkQueuePresentKHR real_vkQueuePresentKHR = nullptr;

namespace present_hook
{
    void init_device(VkDevice device, PFN_vkGetDeviceProcAddr gdpa)
    {
        #warning "present_hook.cpp wird kompiliert"

        LOG_INFO("[AION_VR] present_hook: Initializing device hook");

        real_vkQueuePresentKHR =
            (PFN_vkQueuePresentKHR)gdpa(device, "vkQueuePresentKHR");

        if (!real_vkQueuePresentKHR)
            LOG_ERROR("[AION_VR] Failed to get real vkQueuePresentKHR");
        else
            LOG_INFO("[AION_VR] vkQueuePresentKHR hooked successfully");
    }

    VkResult hook_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo)
    {
        LOG_INFO("[AION_VR] Present intercepted");

        // ------------------------------------------------------------
        // XR: Begin frame
        // ------------------------------------------------------------
        xrbridge::begin_frame();

        // ------------------------------------------------------------
        LOG_INFO("[AION_VR] Present intercepted");
        xrbridge::begin_frame();

        // Callstack dump
        vrdebug::dump_callstack(20);

        // - Kamera-Matrix ersetzen
        // - Stereo-Rendering
        // - Swapchain-Images kopieren
        // ------------------------------------------------------------

        // ------------------------------------------------------------
        // Call the real Present
        // ------------------------------------------------------------
        VkResult res = real_vkQueuePresentKHR(queue, pPresentInfo);

        // ------------------------------------------------------------
        // XR: End frame
        // ------------------------------------------------------------
        xrbridge::end_frame();

        return res;
    }
}
