/*
 * uevent_probe — diagnostic for NETLINK_KOBJECT_UEVENT on the KM6 sensor.
 *
 * Answers one question: can userspace inject a uevent into multicast group 1
 * so that libsl_fp_impl.so:_cust_icon_hbm_status() sees
 * "TRAN_FULL_HBM_EVENT=FULL_HBM_SET"?
 *
 * usage:
 *   uevent_probe listen [seconds] [filter]
 *   uevent_probe send   <payload>
 */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NL_UEVENT 15 /* NETLINK_KOBJECT_UEVENT */

static int open_rx(unsigned int groups)
{
    int fd = socket(AF_NETLINK, SOCK_DGRAM, NL_UEVENT);
    if (fd < 0) {
        perror("socket");
        return -1;
    }
    int rcvbuf = 1 << 20;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUFFORCE, &rcvbuf, sizeof(rcvbuf));

    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = getpid();
    sa.nl_groups = groups;
    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        fprintf(stderr, "bind(groups=0x%x): %s\n", groups, strerror(errno));
        close(fd);
        return -1;
    }
    return fd;
}

static int do_send(const char *payload)
{
    int fd = socket(AF_NETLINK, SOCK_DGRAM, NL_UEVENT);
    if (fd < 0) {
        perror("socket");
        return 1;
    }
    struct sockaddr_nl dst;
    memset(&dst, 0, sizeof(dst));
    dst.nl_family = AF_NETLINK;
    dst.nl_pid = 0;
    dst.nl_groups = 1; /* broadcast to group 1 */

    ssize_t n = sendto(fd, payload, strlen(payload) + 1, 0,
                       (struct sockaddr *)&dst, sizeof(dst));
    if (n < 0) {
        fprintf(stderr, "sendto failed: %s (errno=%d)\n", strerror(errno), errno);
        close(fd);
        return 1;
    }
    printf("sent %zd bytes to group 1: %s\n", n, payload);
    close(fd);
    return 0;
}

static int do_listen(int seconds, const char *filter)
{
    int fd = open_rx(0xFFFFFFFFu);
    if (fd < 0)
        return 1;

    printf("listening %d s, filter=%s\n", seconds, filter ? filter : "(none)");
    fflush(stdout);

    time_t end = time(NULL) + seconds;
    int total = 0, hits = 0;
    char buf[4096];

    while (time(NULL) < end) {
        struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            fprintf(stderr, "recv: %s\n", strerror(errno));
            break;
        }
        buf[n] = 0;
        total++;
        if (!filter || strstr(buf, filter)) {
            hits++;
            printf("--- hit #%d (%zd bytes) ---\n%s\n", hits, n, buf);
            fflush(stdout);
        }
    }
    printf("total uevents=%d, matching=%d\n", total, hits);
    close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "send") == 0) {
        if (argc < 3) {
            fprintf(stderr, "usage: %s send <payload>\n", argv[0]);
            return 2;
        }
        return do_send(argv[2]);
    }
    if (argc >= 2 && strcmp(argv[1], "listen") == 0) {
        int secs = (argc >= 3) ? atoi(argv[2]) : 10;
        return do_listen(secs, argc >= 4 ? argv[3] : NULL);
    }
    fprintf(stderr,
            "usage:\n  %s listen [seconds] [filter]\n  %s send <payload>\n",
            argv[0], argv[0]);
    return 2;
}
