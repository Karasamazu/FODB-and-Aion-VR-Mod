#pragma once
#include <vulkan/vulkan.h>

namespace present_hook
{
    void init_device(VkDevice device, PFN_vkGetDeviceProcAddr gdpa);
    VkResult hook_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
}
