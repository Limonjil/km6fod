#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
int main(void) {
    signal(SIGCHLD, SIG_IGN);
    pid_t p = fork();
    if (p == 0) {
        execl("/system/bin/input", "input", "keyevent", "224", (char*)0);
        _exit(127);
    }
    return 0;
}
