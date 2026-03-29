#include "worker.h"
#include "protocol.h"
#include "io_utils.h"
#include "image_io.h"
#include "filters.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static void send_hello(int worker_id, int write_fd) {
    worker_hello_msg_t hello;
    memset(&hello, 0, sizeof(hello)); // clear

    hello.msg_type = MSG_WORKER_HELLO;
    hello.worker_id = (uint32_t)worker_id;
    hello.pid = getpid();

    if (write_exact(write_fd, &hello, sizeof(hello)) != (ssize_t)sizeof(hello)) {
        perror("worker: write hello");
        exit(EXIT_FAILURE);
    }
}

static void send_result_ok(int worker_id,
                           uint32_t job_id,
                           const char *output_path,
                           int write_fd) {
    result_msg_t result;
    memset(&result, 0, sizeof(result));

    result.msg_type = MSG_RESULT;
    result.worker_id = (uint32_t)worker_id;
    result.job_id = job_id;
    result.status = STATUS_OK;

    strncpy(result.output_path, output_path, MAX_PATH_LEN - 1);
    result.output_path[MAX_PATH_LEN - 1] = '\0';

    if (write_exact(write_fd, &result, sizeof(result)) != (ssize_t)sizeof(result)) {
        perror("worker: write result ok");
        exit(EXIT_FAILURE);
    }
}

static void send_result_fail(int worker_id,
                             uint32_t job_id,
                             const char *error_msg,
                             int write_fd) {
    result_msg_t result;
    memset(&result, 0, sizeof(result));

    result.msg_type = MSG_RESULT;
    result.worker_id = (uint32_t)worker_id;
    result.job_id = job_id;
    result.status = STATUS_FAIL;

    strncpy(result.error_msg, error_msg, MAX_ERR_LEN - 1);
    result.error_msg[MAX_ERR_LEN - 1] = '\0';

    if (write_exact(write_fd, &result, sizeof(result)) != (ssize_t)sizeof(result)) {
        perror("worker: write result fail");
        exit(EXIT_FAILURE);
    }
}

static void process_job(int worker_id,
                        const job_msg_t *job,
                        int write_fd) {
    image_t img;
    memset(&img, 0, sizeof(img));

    if (job == NULL) {
        send_result_fail(worker_id, 0, "job is NULL", write_fd);
        return;
    }

    printf("worker %d: job_id=%u input=%s output=%s\n",
           worker_id, job->job_id, job->input_path, job->output_path);

    if (read_ppm(job->input_path, &img) != 0) {
        send_result_fail(worker_id, job->job_id, "failed to read ppm", write_fd);
        return;
    }

    if (apply_filter(&img, (filter_t)job->filter_type) != 0) {
        free_image(&img);
        send_result_fail(worker_id, job->job_id, "failed to apply filter", write_fd);
        return;
    }

    /* write ppm */
    if (write_ppm(job->output_path, &img) != 0) {
        free_image(&img);
        send_result_fail(worker_id, job->job_id, "failed to write ppm", write_fd);
        return;
    }

    free_image(&img);
    send_result_ok(worker_id, job->job_id, job->output_path, write_fd);
}

void worker_loop(int worker_id, int read_fd, int write_fd) {
    send_hello(worker_id, write_fd);

    while (1) {
        uint32_t msg_type;
        ssize_t n = read_exact(read_fd, &msg_type, sizeof(msg_type));

        if (n <= 0) {
            if (n == 0) {
                fprintf(stderr, "worker %d: EOF from parent\n", worker_id);
                break;
            }
            perror("worker: read msg_type");
            exit(EXIT_FAILURE);
        }

        if (msg_type == MSG_JOB) { // message job
            job_msg_t job;
            memset(&job, 0, sizeof(job));
            job.msg_type = msg_type;

            size_t remaining = sizeof(job) - sizeof(job.msg_type);

            n = read_exact(read_fd,
                           ((char *)&job) + sizeof(job.msg_type),
                           remaining);

            if (n < 0) {
                perror("worker: read job");
                exit(EXIT_FAILURE);
            }

            if (n != (ssize_t)remaining) {
                fprintf(stderr, "worker %d: short read on job\n", worker_id);
                exit(EXIT_FAILURE);
            }

            printf("worker %d: received JOB_MSG\n", worker_id);

            process_job(worker_id, &job, write_fd);

        } else if (msg_type == MSG_SHUTDOWN) { // close the worker
            printf("worker %d: received SHUTDOWN_MSG, exiting normally\n", worker_id);
            break;

        } else { // error
            fprintf(stderr, "worker %d: unknown msg_type=%u\n", worker_id, msg_type);
            exit(EXIT_FAILURE);
        }
    }

    close(read_fd);
    close(write_fd);
    exit(EXIT_SUCCESS);
}