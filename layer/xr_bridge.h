#pragma once

#include <vulkan/vulkan.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <vector>

namespace xrbridge
{
    bool init();
    bool create_swapchain(uint32_t width, uint32_t height);

    void begin_frame();
    void end_frame();

    XrInstance get_instance();
    XrSession get_session();
    XrSwapchain get_swapchain();
    const std::vector<XrSwapchainImageVulkanKHR>& get_swapchain_images();
}
