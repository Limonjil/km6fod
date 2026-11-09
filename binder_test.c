#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
int main(void) {
    int fd = open("/dev/binderfs/binder", O_RDWR);
    if (fd < 0) return 1;
    pid_t p = fork();
    if (p == 0) { execl("/system/bin/input", "input", "keyevent", "224", (char*)0); _exit(127); }
    int st; waitpid(p, &st, 0);
    return 0;
}
