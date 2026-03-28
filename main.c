#include "protocol.h"
#include "io_utils.h"
#include "worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void send_test_job(int write_fd) {
    job_msg_t job;
    memset(&job, 0, sizeof(job));

    job.msg_type = MSG_JOB;
    job.job_id = 1;
    job.filter_type = FILTER_GREY;

    strncpy(job.input_path, "./input/test.ppm", MAX_PATH_LEN - 1);
    job.input_path[MAX_PATH_LEN - 1] = '\0';

    strncpy(job.output_path, "./output/test_grey.ppm", MAX_PATH_LEN - 1);
    job.output_path[MAX_PATH_LEN - 1] = '\0';

    if (write_exact(write_fd, &job, sizeof(job)) != sizeof(job)) {
        perror("parent: write job");
        exit(EXIT_FAILURE);
    }
}

static void send_shutdown(int write_fd) {
    shutdown_msg_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_type = MSG_SHUTDOWN;

    if (write_exact(write_fd, &msg, sizeof(msg)) != sizeof(msg)) {
        perror("parent: write shutdown");
        exit(EXIT_FAILURE);
    }
}

static void recv_hello(int read_fd) {
    worker_hello_msg_t hello;
    ssize_t n = read_exact(read_fd, &hello, sizeof(hello));

    if (n < 0) {
        perror("parent: read hello");
        exit(EXIT_FAILURE);
    }
    if ((size_t)n != sizeof(hello)) {
        fprintf(stderr, "parent: short read on hello\n");
        exit(EXIT_FAILURE);
    }
    if (hello.msg_type != MSG_WORKER_HELLO) {
        fprintf(stderr, "parent: expected WORKER_HELLO, got %u\n", hello.msg_type);
        exit(EXIT_FAILURE);
    }

    printf("parent: received WORKER_HELLO from worker_id=%u pid=%d\n",
           hello.worker_id, hello.pid);
}

static void recv_result(int read_fd) {
    result_msg_t result;
    ssize_t n = read_exact(read_fd, &result, sizeof(result));

    if (n < 0) {
        perror("parent: read result");
        exit(EXIT_FAILURE);
    }
    if ((size_t)n != sizeof(result)) {
        fprintf(stderr, "parent: short read on result\n");
        exit(EXIT_FAILURE);
    }
    if (result.msg_type != MSG_RESULT) {
        fprintf(stderr, "parent: expected RESULT_MSG, got %u\n", result.msg_type);
        exit(EXIT_FAILURE);
    }

    printf("parent: received RESULT_MSG\n");
    printf("parent: worker_id=%u job_id=%u status=%u\n",
           result.worker_id, result.job_id, result.status);

    if (result.status == STATUS_OK) {
        printf("parent: output_path=%s\n", result.output_path);
    } else {
        printf("parent: error_msg=%s\n", result.error_msg);
    }
}

int main(void) {
    int to_worker[2];
    int from_worker[2];

    if (pipe(to_worker) < 0) {
        perror("pipe to_worker");
        return EXIT_FAILURE;
    }
    if (pipe(from_worker) < 0) {
        perror("pipe from_worker");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* child / worker */
        close(to_worker[1]);
        close(from_worker[0]);

        worker_loop(0, to_worker[0], from_worker[1]);
    }

    /* parent */
    close(to_worker[0]);
    close(from_worker[1]);

    recv_hello(from_worker[0]);

    send_test_job(to_worker[1]);

    recv_result(from_worker[0]);

    send_shutdown(to_worker[1]);

    close(to_worker[1]);
    close(from_worker[0]);

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("parent: worker exited with status %d\n", WEXITSTATUS(status));
    } else {
        printf("parent: worker did not exit normally\n");
    }

    return EXIT_SUCCESS;
}