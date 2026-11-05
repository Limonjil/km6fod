/*
 * uinput_wake — probe whether a KEY_WAKEUP emitted through /dev/uinput actually
 * brings the display up on this device.
 *
 * This isolates the mechanism from the fingerprint HAL: run it while the screen
 * is off and watch `dumpsys power | grep mWakefulness`.
 *
 * usage: uinput_wake [keycode]
 */
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static int emit(int fd, int code) {
    struct input_event down;
    memset(&down, 0, sizeof(down));
    down.type = EV_KEY;
    down.code = (unsigned short)code;
    down.value = 1;

    struct input_event syn;
    memset(&syn, 0, sizeof(syn));
    syn.type = EV_SYN;
    syn.code = SYN_REPORT;

    int written = (int)write(fd, &down, sizeof(down));
    written += (int)write(fd, &syn, sizeof(syn));
    down.value = 0;
    written += (int)write(fd, &down, sizeof(down));
    written += (int)write(fd, &syn, sizeof(syn));
    return written;
}

int main(int argc, char** argv) {
    const int code = (argc > 1) ? atoi(argv[1]) : KEY_WAKEUP;

    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open(/dev/uinput): %s\n", strerror(errno));
        return 1;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
        fprintf(stderr, "UI_SET_EVBIT: %s\n", strerror(errno));
    }
    if (ioctl(fd, UI_SET_KEYBIT, code) < 0) {
        fprintf(stderr, "UI_SET_KEYBIT(%d): %s\n", code, strerror(errno));
    }

    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_VIRTUAL;
    setup.id.vendor = 0x0001;
    setup.id.product = 0x0001;
    snprintf(setup.name, sizeof(setup.name), "km6fod-wake-probe");
    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0) {
        fprintf(stderr, "UI_DEV_SETUP: %s\n", strerror(errno));
    }
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        fprintf(stderr, "UI_DEV_CREATE: %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    usleep(60 * 1000);
    const int written = emit(fd, code);
    usleep(40 * 1000);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    printf("emitted keycode %d (%d bytes) via /dev/uinput\n", code, written);
    return 0;
}
