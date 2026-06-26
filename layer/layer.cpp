#include "layer.h"
#include <cstdio>
#include <cstring>
#include "present_hook.h"
#include "xr_bridge.h"
#include "camera_hook.h"
#include <dlfcn.h>

// Forward-Deklaration, damit der Compiler camerahook kennt
namespace camerahook {
    bool init(uintptr_t moduleBase);
}



uintptr_t get_module_base()
{
    Dl_info info;
    if (dladdr((void*)get_module_base, &info))
        return (uintptr_t)info.dli_fbase;

    return 0;
}

std::unordered_map<VkInstance, InstanceDispatchTable> g_instanceDispatch;
std::unordered_map<VkDevice, DeviceDispatchTable> g_deviceDispatch;

static void Log(const char* msg) {
    FILE* f = fopen("/tmp/aion_vr_layer.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

extern "C" VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(uint32_t* pVersion) {
    Log("AION-VR: vkNegotiateLoaderLayerInterfaceVersion");
    if (*pVersion > 2) *pVersion = 2;
    return VK_SUCCESS;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetInstanceProcAddr(VkInstance instance, const char* pName)
{
    if (!strcmp(pName, "vkCreateInstance"))
        return (PFN_vkVoidFunction)Hook_vkCreateInstance;

    if (!strcmp(pName, "vkCreateDevice"))
        return (PFN_vkVoidFunction)Hook_vkCreateDevice;

    return nullptr;
}

extern "C" VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
vkGetDeviceProcAddr(VkDevice device, const char* pName)
{
    if (!strcmp(pName, "vkQueuePresentKHR"))
        return (PFN_vkVoidFunction)Hook_vkQueuePresentKHR;

    return nullptr;
}

VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateInstance(
    const VkInstanceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkInstance* pInstance)
{
    Log("AION-VR: Hook_vkCreateInstance");

    auto fp = g_instanceDispatch[VK_NULL_HANDLE].CreateInstance;
    xrbridge::init();
    VkResult res = fp(pCreateInfo, pAllocator, pInstance);

    if (res == VK_SUCCESS) {
        InstanceDispatchTable table{};
        table.GetInstanceProcAddr =
            g_instanceDispatch[VK_NULL_HANDLE].GetInstanceProcAddr;

        table.CreateDevice = (PFN_vkCreateDevice)
            table.GetInstanceProcAddr(*pInstance, "vkCreateDevice");

        g_instanceDispatch[*pInstance] = table;
    }
    uintptr_t moduleBase = get_module_base();
    camerahook::init(moduleBase);


    return res;
}

VKAPI_ATTR VkResult VKAPI_CALL Hook_vkCreateDevice(
    VkPhysicalDevice physicalDevice,
    const VkDeviceCreateInfo* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDevice* pDevice)
{
    Log("AION-VR: Hook_vkCreateDevice");

    auto fp = g_instanceDispatch.begin()->second.CreateDevice;
    VkResult res = fp(physicalDevice, pCreateInfo, pAllocator, pDevice);

    if (res == VK_SUCCESS) {
        DeviceDispatchTable table{};
        table.GetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)
            g_instanceDispatch.begin()->second.GetInstanceProcAddr(
                VK_NULL_HANDLE, "vkGetDeviceProcAddr");

        present_hook::init_device(*pDevice, table.GetDeviceProcAddr);
        table.QueuePresentKHR = present_hook::hook_vkQueuePresentKHR;

        g_deviceDispatch[*pDevice] = table;
    }

    return res;
}

VKAPI_ATTR VkResult VKAPI_CALL Hook_vkQueuePresentKHR(
    VkQueue queue,
    const VkPresentInfoKHR* pPresentInfo)
{
    Log("AION-VR: Present intercepted!");

    auto device = g_deviceDispatch.begin()->first;
    auto fp = g_deviceDispatch[device].QueuePresentKHR;

    return fp(queue, pPresentInfo);
}
