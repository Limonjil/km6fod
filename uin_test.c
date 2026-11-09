#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <linux/input.h>
int main(void) {
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) return 1;
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, KEY_WAKEUP);
    struct uinput_setup us;
    memset(&us, 0, sizeof us);
    us.id.bustype = BUS_VIRTUAL; us.id.vendor = 0x6b36; us.id.product = 1;
    strcpy(us.name, "km6wake");
    if (ioctl(fd, UI_DEV_SETUP, &us) < 0) return 2;
    if (ioctl(fd, UI_DEV_CREATE) < 0) return 3;
    usleep(300000);
    struct input_event ev;
    memset(&ev, 0, sizeof ev); ev.type = EV_KEY; ev.code = KEY_WAKEUP; ev.value = 1; write(fd, &ev, sizeof ev);
    memset(&ev, 0, sizeof ev); ev.type = EV_SYN; write(fd, &ev, sizeof ev);
    usleep(60000);
    memset(&ev, 0, sizeof ev); ev.type = EV_KEY; ev.code = KEY_WAKEUP; ev.value = 0; write(fd, &ev, sizeof ev);
    memset(&ev, 0, sizeof ev); ev.type = EV_SYN; write(fd, &ev, sizeof ev);
    usleep(150000);
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
