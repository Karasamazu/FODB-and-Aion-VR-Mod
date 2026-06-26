#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define SHM_NAME "/aion_vr_state"

typedef struct {
    float m[16];
} Mat4;

typedef struct {
    uint64_t seq;
    Mat4 view_left;
    Mat4 view_right;
    Mat4 proj_left;
    Mat4 proj_right;
} VrSharedState;


// ------------------------------------------------------------
// Shared Memory
// ------------------------------------------------------------
static VrSharedState* init_shared_memory() {
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("shm_open");
        return NULL;
    }

    if (ftruncate(fd, sizeof(VrSharedState)) != 0) {
        perror("ftruncate");
        return NULL;
    }

    void* ptr = mmap(NULL, sizeof(VrSharedState),
                     PROT_READ | PROT_WRITE,
                     MAP_SHARED, fd, 0);

    close(fd);

    if (ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    VrSharedState* state = (VrSharedState*)ptr;
    memset(state, 0, sizeof(VrSharedState));
    return state;
}


// ------------------------------------------------------------
// Convert Pose → View Matrix
// ------------------------------------------------------------
static void convert_pose_to_view(const XrPosef* pose, Mat4* out) {
    float x = pose->orientation.x;
    float y = pose->orientation.y;
    float z = pose->orientation.z;
    float w = pose->orientation.w;

    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, xz = x * z, yz = y * z;
    float wx = w * x, wy = w * y, wz = w * z;

    float R[16] = {
        1 - 2*(yy + zz), 2*(xy - wz),     2*(xz + wy),     0,
        2*(xy + wz),     1 - 2*(xx + zz), 2*(yz - wx),     0,
        2*(xz - wy),     2*(yz + wx),     1 - 2*(xx + yy), 0,
        0,               0,               0,               1
    };

    float tx = -pose->position.x;
    float ty = -pose->position.y;
    float tz = -pose->position.z;

    memcpy(out->m, R, sizeof(R));
    out->m[12] = tx;
    out->m[13] = ty;
    out->m[14] = tz;
}


// ------------------------------------------------------------
// Convert FOV → Projection Matrix
// ------------------------------------------------------------
static void convert_fov_to_proj(const XrFovf* fov, float nearZ, float farZ, Mat4* out) {
    float tanLeft  = tanf(fov->angleLeft);
    float tanRight = tanf(fov->angleRight);
    float tanUp    = tanf(fov->angleUp);
    float tanDown  = tanf(fov->angleDown);

    float w = tanRight - tanLeft;
    float h = tanUp - tanDown;

    float A = (tanRight + tanLeft) / w;
    float B = (tanUp + tanDown) / h;

    float C = -(farZ + nearZ) / (farZ - nearZ);
    float D = -(2 * farZ * nearZ) / (farZ - nearZ);

    float m[16] = {
        2 / w, 0,     A, 0,
        0,     2 / h, B, 0,
        0,     0,     C, D,
        0,     0,    -1, 0
    };

    memcpy(out->m, m, sizeof(m));
}


// ------------------------------------------------------------
// MAIN (Headless OpenXR Tracking)
// ------------------------------------------------------------
int main() {
    VrSharedState* shm = init_shared_memory();
    if (!shm) {
        printf("Failed to init shared memory\n");
        return 1;
    }

    // --- OpenXR Instance ---
    XrInstance instance;
    XrInstanceCreateInfo ci = {
        .type = XR_TYPE_INSTANCE_CREATE_INFO,
        .applicationInfo = {
            .applicationName = "PoseReaderHeadless",
            .applicationVersion = 1,
            .engineName = "None",
            .engineVersion = 1,
            .apiVersion = XR_CURRENT_API_VERSION
        }
    };

    if (xrCreateInstance(&ci, &instance) != XR_SUCCESS) {
        printf("xrCreateInstance failed\n");
        return 1;
    }

    // --- System ---
    XrSystemId sysId;
    XrSystemGetInfo si = {
        .type = XR_TYPE_SYSTEM_GET_INFO,
        .formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY
    };

    if (xrGetSystem(instance, &si, &sysId) != XR_SUCCESS) {
        printf("xrGetSystem failed\n");
        return 1;
    }

    // --- Headless Session ---
    XrSession session;
    XrSessionCreateInfo sci = {
        .type = XR_TYPE_SESSION_CREATE_INFO,
        .next = NULL,
        .systemId = sysId
    };

    if (xrCreateSession(instance, &sci, &session) != XR_SUCCESS) {
        printf("xrCreateSession failed\n");
        return 1;
    }

    // --- View Config ---
    uint32_t viewCount = 0;
    xrEnumerateViewConfigurationViews(instance, sysId,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, NULL);

    XrViewConfigurationView views[2];
    for (uint32_t i = 0; i < viewCount; i++)
        views[i].type = XR_TYPE_VIEW_CONFIGURATION_VIEW;

    xrEnumerateViewConfigurationViews(instance, sysId,
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        viewCount, &viewCount, views);

    XrView xrViews[2];
    for (int i = 0; i < 2; i++)
        xrViews[i].type = XR_TYPE_VIEW;

    // --- Headless Loop ---
    while (1) {
        XrViewLocateInfo vli = {
            .type = XR_TYPE_VIEW_LOCATE_INFO,
            .viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
            .displayTime = 0,          // headless
            .space = XR_NULL_HANDLE    // headless
        };

        uint32_t outCount;
        XrViewState viewState = { .type = XR_TYPE_VIEW_STATE };

        xrLocateViews(
            session,
            &vli,
            &viewState,
            2,
            &outCount,
            xrViews
        );



        convert_pose_to_view(&xrViews[0].pose, &shm->view_left);
        convert_pose_to_view(&xrViews[1].pose, &shm->view_right);

        convert_fov_to_proj(&xrViews[0].fov, 0.05f, 1000.0f, &shm->proj_left);
        convert_fov_to_proj(&xrViews[1].fov, 0.05f, 1000.0f, &shm->proj_right);

        shm->seq++;

        usleep(1000); // 1ms
    }

    return 0;
}
