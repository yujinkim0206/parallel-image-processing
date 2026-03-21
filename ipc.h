#ifndef IPC_H
#define IPC_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#define MSG_TASK   1
#define MSG_RESULT 2
#define MSG_QUIT   3

typedef struct {
    uint32_t type;        // MSG_TASK
    uint32_t job_id;
    uint32_t start_row;
    uint32_t end_row;
    uint32_t width;
    uint32_t data_len;
    uint32_t filter_type;
} job_msg_t;

typedef struct {
    uint32_t type;        // MSG_RESULT
    uint32_t job_id;
    uint32_t start_row;
    uint32_t end_row;
    uint32_t data_len;
    uint32_t status;      // 0 = success, nonzero = failure
} result_msg_t;

typedef struct {
    uint32_t type;        // MSG_QUIT
} quit_msg_t;

ssize_t write_all(int fd, const void *buf, size_t count);
ssize_t read_all(int fd, void *buf, size_t count);

#endif