/*
 * hbm_probe — call the vendor engine's own power/HBM entry points.
 *
 * Stock drives the panel from the kernel (tran_fp). The engine exposes the same
 * path to its HAL: silfp_impl_wakelock (SIFP_IOC_WAKELOCK) and
 * silfp_cust_set_hbm_mode. This probe calls them directly, with our own userspace
 * HBM disabled, to see whether the KERNEL can wake and light the panel by itself.
 *
 * usage: hbm_probe wake|hbm1|hbm0
 */
#include <dlfcn.h>
#include <cstdio>
#include <cstring>

namespace {
constexpr const char* kEngine = "/vendor/lib64/libsl_fp_impl.so";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s wake|hbm1|hbm0\n", argv[0]);
        return 2;
    }
    void* handle = ::dlopen(kEngine, RTLD_NOW | RTLD_LOCAL);
    if (handle == nullptr) {
        std::fprintf(stderr, "dlopen failed: %s\n", ::dlerror());
        return 1;
    }
    const char* what = argv[1];
    if (std::strcmp(what, "wake") == 0) {
        auto impl_wake =
            reinterpret_cast<int (*)(int)>(::dlsym(handle, "silfp_impl_wakelock"));
        auto dev_wake =
            reinterpret_cast<int (*)(int)>(::dlsym(handle, "silfp_dev_wakelock"));
        std::printf("symbols: silfp_impl_wakelock=%s silfp_dev_wakelock=%s\n",
                    impl_wake ? "yes" : "no", dev_wake ? "yes" : "no");
        if (dev_wake != nullptr) {
            std::printf("silfp_dev_wakelock(1) -> %d\n", dev_wake(1));
        }
        if (impl_wake != nullptr) {
            std::printf("silfp_impl_wakelock(1) -> %d\n", impl_wake(1));
        }
        return 0;
    }
    auto set_hbm = reinterpret_cast<int (*)(int)>(::dlsym(handle, "silfp_cust_set_hbm_mode"));
    if (set_hbm == nullptr) {
        std::fprintf(stderr, "no silfp_cust_set_hbm_mode\n");
        return 1;
    }
    const int on = (std::strcmp(what, "hbm1") == 0) ? 1 : 0;
    std::printf("silfp_cust_set_hbm_mode(%d) -> %d\n", on, set_hbm(on));
    return 0;
}
