
#include "xr_bridge.h"
#include "util/log.h"
#include <cstring>


namespace xrbridge
{
    static XrInstance xrInstance = XR_NULL_HANDLE;
    static XrSession xrSession = XR_NULL_HANDLE;
    static XrSystemId xrSystemId = XR_NULL_SYSTEM_ID;

    static XrSwapchain xrSwapchain = XR_NULL_HANDLE;
    static std::vector<XrSwapchainImageVulkanKHR> xrSwapchainImages;

    static bool xr_ok(XrResult r, const char* msg)
    {
        if (r != XR_SUCCESS) {
            LOG_ERROR("[XR] %s failed: %d", msg, r);
            return false;
        }
        return true;
    }

    bool init()
    {
        LOG_INFO("[XR] Initializing OpenXR...");

        // ------------------------------------------------------------
        // 1) Instance
        // ------------------------------------------------------------
        XrInstanceCreateInfo ici{XR_TYPE_INSTANCE_CREATE_INFO};
        strcpy(ici.applicationInfo.applicationName, "AionVR");
        ici.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;

        if (!xr_ok(xrCreateInstance(&ici, &xrInstance), "xrCreateInstance"))
            return false;

        // ------------------------------------------------------------
        // 2) System (HMD)
        // ------------------------------------------------------------
        XrSystemGetInfo sgi{XR_TYPE_SYSTEM_GET_INFO};
        sgi.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;

        if (!xr_ok(xrGetSystem(xrInstance, &sgi, &xrSystemId), "xrGetSystem"))
            return false;

        LOG_INFO("[XR] System ID: %llu", xrSystemId);

        // ------------------------------------------------------------
        // 3) Session (Vulkan)
        // ------------------------------------------------------------
        XrGraphicsBindingVulkanKHR gb{XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR};

        // Vulkan handles werden später vom Present-Hook gesetzt
        // gb.device = ...
        // gb.queueFamilyIndex = ...
        // gb.queueIndex = ...

        XrSessionCreateInfo sci{XR_TYPE_SESSION_CREATE_INFO};
        sci.systemId = xrSystemId;
        sci.next = &gb;

        if (!xr_ok(xrCreateSession(xrInstance, &sci, &xrSession), "xrCreateSession"))
            return false;

        LOG_INFO("[XR] Session created.");
        return true;
    }

    bool create_swapchain(uint32_t width, uint32_t height)
    {
        LOG_INFO("[XR] Creating swapchain %ux%u...", width, height);

        XrSwapchainCreateInfo sci{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        sci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        sci.format = VK_FORMAT_R8G8B8A8_SRGB;
        sci.sampleCount = 1;
        sci.width = width;
        sci.height = height;
        sci.faceCount = 1;
        sci.arraySize = 2;
        sci.mipCount = 1;

        if (!xr_ok(xrCreateSwapchain(xrSession, &sci, &xrSwapchain), "xrCreateSwapchain"))
            return false;

        uint32_t count = 0;
        xrEnumerateSwapchainImages(xrSwapchain, 0, &count, nullptr);

        xrSwapchainImages.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR});

        if (!xr_ok(xrEnumerateSwapchainImages(
                xrSwapchain, count, &count,
                (XrSwapchainImageBaseHeader*)xrSwapchainImages.data()),
            "xrEnumerateSwapchainImages"))
            return false;

        LOG_INFO("[XR] Swapchain created with %u images.", count);
        return true;
    }

    void begin_frame()
    {
        XrFrameBeginInfo fbi{XR_TYPE_FRAME_BEGIN_INFO};
        xrBeginFrame(xrSession, &fbi);
    }

    void end_frame()
    {
        XrFrameEndInfo fei{XR_TYPE_FRAME_END_INFO};
        fei.displayTime = 0;
        fei.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        fei.layerCount = 0;
        fei.layers = nullptr;

        xrEndFrame(xrSession, &fei);
    }

    XrInstance get_instance() { return xrInstance; }
    XrSession get_session() { return xrSession; }
    XrSwapchain get_swapchain() { return xrSwapchain; }
    const std::vector<XrSwapchainImageVulkanKHR>& get_swapchain_images() { return xrSwapchainImages; }
}
