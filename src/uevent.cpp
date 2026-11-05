// SPDX-License-Identifier: GPL-2.0-only

#include "uevent.h"

#include <linux/netlink.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <unistd.h>

#include "logging.h"

namespace km6fod {
namespace {

constexpr int kNetlinkKobjectUevent = 15;
constexpr unsigned int kGroupOne = 1;

}  // namespace

UeventInjector::~UeventInjector() {
    std::lock_guard<std::mutex> lock(mutex_);
    closeLocked();
}

void UeventInjector::closeLocked() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

bool UeventInjector::send(const char* event_line) {
    if (event_line == nullptr) {
        return false;
    }

    // A well-formed uevent datagram: NUL-separated property lines. Keeping the
    // canonical shape makes the packet look like a real kernel event to
    // anything else listening on group 1.
    static const char kHeaderFixed[] = "change@/devices/virtual/fod";
    char buffer[256];
    const int length = std::snprintf(
        buffer, sizeof(buffer),
        "%s%cACTION=change%cDEVPATH=/devices/virtual/fod%cSUBSYSTEM=fod%c%s%cSEQNUM=%u%c",
        kHeaderFixed, '\0', '\0', '\0', '\0', event_line, '\0',
        static_cast<unsigned int>(std::time(nullptr) & 0xFFFFu), '\0');
    if (length <= 0 || static_cast<size_t>(length) >= sizeof(buffer)) {
        LOGE("uevent: payload truncated");
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (fd_ < 0) {
        fd_ = ::socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, kNetlinkKobjectUevent);
        if (fd_ < 0) {
            LOGE("uevent: socket() failed: %s", std::strerror(errno));
            return false;
        }
        LOGI("uevent: injector ready (fd=%d)", fd_);
    }

    struct sockaddr_nl destination;
    std::memset(&destination, 0, sizeof(destination));
    destination.nl_family = AF_NETLINK;
    destination.nl_pid = 0;  // kernel/default peer
    destination.nl_groups = kGroupOne;

    const ssize_t sent = ::sendto(fd_, buffer, static_cast<size_t>(length), 0,
                                  reinterpret_cast<struct sockaddr*>(&destination),
                                  sizeof(destination));
    if (sent != static_cast<ssize_t>(length)) {
        LOGE("uevent: sendto(%s) failed: %s", event_line, std::strerror(errno));
        // A stale socket must not poison every later attempt.
        closeLocked();
        return false;
    }
    LOGI("uevent: injected %s (%zd bytes)", event_line, sent);
    return true;
}

bool UeventInjector::sendFullHbmSet() {
    return send("TRAN_FULL_HBM_EVENT=FULL_HBM_SET");
}

bool UeventInjector::sendFullHbmClear() {
    return send("TRAN_FULL_HBM_EVENT=FULL_HBM_CLEAR");
}

}  // namespace km6fod
