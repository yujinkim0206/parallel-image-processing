#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stddef.h>
#include <sys/types.h>

/*
 * Reads exactly n bytes unless EOF or error occurs.
 *
 * Return value:
 *   n    : success, exactly n bytes read
 *   0    : EOF encountered before any byte was read
 *   -1   : error
 *   >0<n : partial read before EOF
 */
ssize_t read_exact(int fd, void *buf, size_t n);

/*
 * Writes exactly n bytes unless error occurs.
 *
 * Return value:
 *   n  : success, exactly n bytes written
 *   -1 : error
 */
ssize_t write_exact(int fd, const void *buf, size_t n);

#endif /* IO_UTILS_H */