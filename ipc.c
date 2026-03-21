#include <unistd.h>
#include "ipc.h"

ssize_t write_all(int fd, const void *buf, size_t count) {
    size_t total = 0;
    const char *p = buf;

    while (total < count) {
        ssize_t n = write(fd, p + total, count - total);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        total += (size_t)n;
    }

    return (ssize_t)total;
}

ssize_t read_all(int fd, void *buf, size_t count) {
    size_t total = 0;
    char *p = buf;

    while (total < count) {
        ssize_t n = read(fd, p + total, count - total);
        if (n < 0) {
            return -1;
        }
        if (n == 0) {
            return -1;
        }
        total += (size_t)n;
    }

    return (ssize_t)total;
}