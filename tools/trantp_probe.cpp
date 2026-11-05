/*
 * trantp_probe — call vendor.hardware.trantp.ITrantp/default.
 *
 * The vendor touch service is what enables the FOD wake gesture on stock. Its
 * AIDL interface was recovered from the binary:
 *
 *   code 1 -> method(int32)
 *   code 2 -> method(int32 cmd, String value)   <-- the "opration" call
 *   code 3, code 4 -> ... (not used here)
 *
 * This tool issues a caller-supplied command so the service logs
 * "Trantp opration path= ..." and we learn which node each command reaches.
 *
 * usage: trantp_probe <code> <cmd> [value]
 */
#include <android/binder_ibinder.h>
#include <android/binder_parcel.h>
#include <android/binder_status.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

// The NDK sysroot does not declare the service-manager lookup.
extern "C" AIBinder* AServiceManager_getService(const char* instance);

namespace {

constexpr const char* kInstance = "vendor.hardware.trantp.ITrantp/default";
constexpr const char* kDescriptor = "vendor.hardware.trantp.ITrantp";

// Minimal local class: the NDK refuses prepareTransaction on a remote binder
// unless an interface class has been associated with it.
void* onCreate(void* /*args*/) {
    return nullptr;
}
void onDestroy(void* /*userData*/) {}
binder_status_t onTransact(AIBinder* /*binder*/, transaction_code_t /*code*/,
                           const AParcel* /*in*/, AParcel* /*out*/) {
    return STATUS_UNKNOWN_TRANSACTION;
}

}  // namespace


// 0 -> the panel is dark (asleep or dozing), >0 -> lit, <0 -> unknown.
int panelBacklight() {
    const int fd = ::open("/sys/class/leds/lcd-backlight/brightness", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return -1;
    }
    char buf[32] = {};
    const ssize_t n = ::read(fd, buf, sizeof(buf) - 1);
    ::close(fd);
    return (n > 0) ? std::atoi(buf) : -1;
}

int main(int argc, char** argv) {
    const bool keep = (argc == 2 && std::strcmp(argv[1], "--keep") == 0);
    if (!keep && argc < 3) {
        std::fprintf(stderr, "usage: %s <code> <cmd> [value]\n", argv[0]);
        return 2;
    }
    const transaction_code_t code =
        static_cast<transaction_code_t>(keep ? 0 : std::atoi(argv[1]));
    const int32_t cmd = keep ? 0 : std::atoi(argv[2]);
    const char* value = (argc >= 4) ? argv[3] : "";

    AIBinder* service = AServiceManager_getService(kInstance);
    if (service == nullptr) {
        std::fprintf(stderr, "service %s not found\n", kInstance);
        return 1;
    }

    AIBinder_Class* clazz =
        AIBinder_Class_define(kDescriptor, &onCreate, &onDestroy, &onTransact);
    if (clazz == nullptr || !AIBinder_associateClass(service, clazz)) {
        std::fprintf(stderr, "cannot associate class %s\n", kDescriptor);
        return 1;
    }


    if (keep) {
        std::printf("keep: gesture keeper started\n");
        bool enabled = false;
        for (;;) {
            const int backlight = panelBacklight();
            if (backlight == 0) {
                if (!enabled) {
                    std::printf("keep: panel dark, re-enabling the gesture\n");
                    enabled = true;
                }
                const int fd = ::open("/proc/gesture_function", O_WRONLY | O_CLOEXEC);
                if (fd >= 0) {
                    const ssize_t n = ::write(fd, "fp1\n", 4);
                    ::close(fd);
                    std::fprintf(stderr, "keep: wrote fp1 (%zd)\n", n);
                } else {
                    std::fprintf(stderr, "keep: open /proc/gesture_function failed\n");
                }
            } else if (backlight > 0 && enabled) {
                std::printf("keep: panel lit, standby\n");
                enabled = false;
            }
            ::usleep(2 * 1000 * 1000);
        }
    }

    AParcel* in = nullptr;
    binder_status_t status = AIBinder_prepareTransaction(service, &in);
    if (status != STATUS_OK || in == nullptr) {
        std::fprintf(stderr, "prepareTransaction failed: %d\n", status);
        return 1;
    }

    status = AParcel_writeInt32(in, cmd);
    if (status == STATUS_OK) {
        status = AParcel_writeString(in, value, static_cast<int32_t>(std::strlen(value)));
    }
    if (status != STATUS_OK) {
        std::fprintf(stderr, "write failed: %d\n", status);
        AParcel_delete(in);
        return 1;
    }

    AParcel* out = nullptr;
    status = AIBinder_transact(service, code, &in, &out, 0 /* synchronous */);
    std::printf("transact(code=%u, cmd=%d, value='%s') -> %d\n", static_cast<unsigned>(code), cmd,
                value, status);
    if (status != STATUS_OK) {
        return 1;
    }

    if (out != nullptr) {
        AStatus* parcel_status = nullptr;
        if (AParcel_readStatusHeader(out, &parcel_status) == STATUS_OK && parcel_status != nullptr) {
            const char* message = AStatus_getMessage(parcel_status);
            std::printf("  reply status: %d (ok=%d) message='%s'\n",
                        AStatus_getStatus(parcel_status), AStatus_isOk(parcel_status) ? 1 : 0,
                        message != nullptr ? message : "");
            AStatus_delete(parcel_status);
        }
        AParcel_delete(out);
    }
    return 0;
}
