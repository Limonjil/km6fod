// uinput_persist — does a uinput device that is created once keep waking the
// display on every later emit, or only on the first one?
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <linux/input-event-codes.h>

static int emit(int fd, const char *tag) {
    struct input_event ev[2];
    memset(ev, 0, sizeof(ev));
    ev[0].type = EV_KEY; ev[0].code = KEY_WAKEUP; ev[0].value = 1;
    ev[1].type = EV_SYN; ev[1].code = SYN_REPORT; ev[1].value = 0;
    int n = (int)write(fd, ev, sizeof(ev));
    memset(ev, 0, sizeof(ev));
    ev[0].type = EV_KEY; ev[0].code = KEY_WAKEUP; ev[0].value = 0;
    ev[1].type = EV_SYN; ev[1].code = SYN_REPORT; ev[1].value = 0;
    n += (int)write(fd, ev, sizeof(ev));
    printf("%s emitted (%d bytes)\n", tag, n);
    fflush(stdout);
    return n;
}

int main(int argc, char **argv) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) { perror("open"); return 1; }
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_WAKEUP);
    struct uinput_setup setup;
    memset(&setup, 0, sizeof(setup));
    setup.id.bustype = BUS_VIRTUAL;
    setup.id.vendor = 0x0001;
    setup.id.product = 0x0001;
    snprintf(setup.name, sizeof(setup.name), "km6fod-persist");
    if (ioctl(fd, UI_DEV_SETUP, &setup) < 0) { perror("setup"); return 1; }
    if (ioctl(fd, UI_DEV_CREATE) < 0) { perror("create"); return 1; }
    usleep(300 * 1000);

    emit(fd, "first");
    const int wait = (argc > 1) ? atoi(argv[1]) : 15;
    printf("sleeping %d s\n", wait); fflush(stdout);
    sleep(wait);
    emit(fd, "second");
    sleep(3);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
