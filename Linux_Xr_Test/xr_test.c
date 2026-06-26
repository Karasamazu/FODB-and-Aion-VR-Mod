#include <stdio.h>
#include <openxr/openxr.h>

int main() {
    // 1. Instance erstellen
    XrInstance instance;
    XrInstanceCreateInfo createInfo = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = {
            .applicationName = "XRTest",
            .applicationVersion = 1,
            .engineName = "None",
            .engineVersion = 1,
            .apiVersion = XR_CURRENT_API_VERSION
        }
    };

    XrResult r = xrCreateInstance(&createInfo, &instance);
    if (r != XR_SUCCESS) {
        printf("xrCreateInstance failed: %d\n", r);
        return 1;
    }
    printf("xrCreateInstance OK\n");

    // 2. System (HMD) abfragen
    XrSystemId systemId;
    XrSystemGetInfo sysInfo = {
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
    };

    r = xrGetSystem(instance, &sysInfo, &systemId);
    if (r != XR_SUCCESS) {
        printf("xrGetSystem failed: %d\n", r);
        return 1;
    }
    printf("xrGetSystem OK, systemId = %lu\n", systemId);

    // 3. View-Konfigurationen prüfen
    uint32_t viewCount = 0;
    r = xrEnumerateViewConfigurations(instance, systemId, 0, &viewCount, NULL);
    if (r != XR_SUCCESS) {
        printf("xrEnumerateViewConfigurations failed: %d\n", r);
        return 1;
    }

    printf("View configurations available: %u\n", viewCount);

    return 0;
}
