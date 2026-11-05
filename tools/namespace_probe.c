/*
 * namespace_probe — checks whether a given binary path can reach the vendor
 * fingerprint libraries through the dynamic linker namespace.
 *
 * This answers the question that decides where the HAL binary must be executed
 * from: the vendor engine lives in /vendor/lib64 and the default linker
 * namespace of a /data process is not allowed to load it.
 *
 * usage: namespace_probe
 */
#include <dlfcn.h>
#include <stdio.h>

static void probe(const char* what, const char* name) {
    void* handle = dlopen(name, RTLD_NOW);
    if (handle == NULL) {
        printf("FAIL %-10s %-45s %s\n", what, name, dlerror());
        return;
    }
    printf("OK   %-10s %s\n", what, name);

    if (what[0] == 'h') {
        void* sym = dlsym(handle, "HMI");
        printf("     %-10s HMI -> %s\n", "symbol", sym ? "found" : "MISSING");
    }
}

int main(int argc, char** argv) {
    printf("probe from %s\n", argc > 1 ? argv[1] : "(unknown path)");
    probe("hal", "fingerprint.silead.default.so");
    probe("halabs", "/vendor/lib64/hw/fingerprint.silead.default.so");
    probe("engine", "libsl_fp_impl.so");
    probe("engabs", "/vendor/lib64/libsl_fp_impl.so");
    return 0;
}
