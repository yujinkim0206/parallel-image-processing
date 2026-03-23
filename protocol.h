#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <sys/types.h>

#define MAX_PATH_LEN 256
#define MAX_ERR_LEN  128

typedef enum {
    FILTER_GREY = 1,
    FILTER_BLUR = 2,
    FILTER_EDGE = 3
} filter_t;

typedef enum {
    MSG_WORKER_HELLO = 1,
    MSG_JOB = 2,
    MSG_RESULT = 3,
    MSG_SHUTDOWN = 4
} msg_type_t;

typedef enum {
    STATUS_OK = 0,
    STATUS_FAIL = 1
} status_t;

typedef enum {
    FILE_PPM = 1,
    FILE_BMP = 2
} file_type_t;

/*
 * Worker -> Parent
 * Sent once after worker initialization is complete.
 */
typedef struct {
    uint32_t msg_type;   /* MSG_WORKER_HELLO */
    uint32_t worker_id;
    pid_t    pid;
} worker_hello_msg_t;

/*
 * Parent -> Worker
 * Assigns one image-processing job.
 */
typedef struct {
    uint32_t msg_type;                 /* MSG_JOB */
    uint32_t job_id;
    uint32_t filter_type;              /* FILTER_GREY / FILTER_BLUR / FILTER_EDGE */
    uint32_t file_type;                /* FILE_PPM / FILE_BMP */
    char     input_path[MAX_PATH_LEN]; /* null-terminated */
    char     output_path[MAX_PATH_LEN];/* null-terminated */
} job_msg_t;

/*
 * Worker -> Parent
 * Reports success/failure of one completed job.
 */
typedef struct {
    uint32_t msg_type;                  /* MSG_RESULT */
    uint32_t worker_id;
    uint32_t job_id;
    uint32_t status;                    /* STATUS_OK / STATUS_FAIL */
    char     output_path[MAX_PATH_LEN]; /* meaningful when status == STATUS_OK */
    char     error_msg[MAX_ERR_LEN];    /* meaningful when status == STATUS_FAIL */
} result_msg_t;

/*
 * Parent -> Worker
 * Signals graceful shutdown.
 */
typedef struct {
    uint32_t msg_type;   /* MSG_SHUTDOWN */
} shutdown_msg_t;

#endif /* PROTOCOL_H */