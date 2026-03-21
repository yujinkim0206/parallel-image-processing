#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
// #include <sys/types.h> if using ssize_t

// message types
typedef enum {
    MSG_TASK   = 1,
    MSG_RESULT = 2,
    MSG_QUIT   = 3
} msg_type_t;

// filter types
typedef enum {
    FILTER_GREY = 1,
    FILTER_BLUR = 2,
    FILTER_EDGE = 3
} filter_type_t;

// // status codes
// typedef enum {
//     STATUS_SUCCESS = 0,
//     STATUS_FAILURE = 1
// } status_t;

#define PIXEL_SIZE 3   // 3 bytes per pixel

/*
 * Parent -> Worker
 * Header is followed by data_len bytes of raw pixel data.
 */
typedef struct {
    uint32_t type;
    uint32_t job_id;
    uint32_t start_row;
    uint32_t end_row;
    uint32_t width;
    uint32_t data_len;
    uint32_t filter_type;
} job_msg_t;

/*
 * Worker -> Parent
 * Header is followed by data_len bytes of processed pixel data.
 */
typedef struct {
    uint32_t type;
    uint32_t job_id;
    uint32_t start_row;
    uint32_t end_row;
    uint32_t data_len;
    uint32_t status;
} result_msg_t;

/*
 * Parent -> Worker
 * No payload follows.
 */
typedef struct {
    uint32_t type;
} quit_msg_t;

// /* Robust I/O helpers */ - optional
// ssize_t read_all(int fd, void *buf, size_t count);
// ssize_t write_all(int fd, const void *buf, size_t count);

#endif