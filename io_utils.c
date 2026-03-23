#include "io_utils.h"

#include <errno.h>
#include <unistd.h>

ssize_t read_exact(int fd, void *buf, size_t n) {
    size_t total = 0;
    char *p = (char *)buf;

    while (total < n) {
        ssize_t bytes_read = read(fd, p + total, n - total);

        if (bytes_read == 0) {
            /* EOF */
            if (total == 0) {
                return 0;
            }
            return (ssize_t)total;
        }

        if (bytes_read < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        total += (size_t)bytes_read;
    }

    return (ssize_t)total;
}

ssize_t write_exact(int fd, const void *buf, size_t n) {
    size_t total = 0;
    const char *p = (const char *)buf;

    while (total < n) {
        ssize_t bytes_written = write(fd, p + total, n - total);

        if (bytes_written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }

        total += (size_t)bytes_written;
    }

    return (ssize_t)total;
}