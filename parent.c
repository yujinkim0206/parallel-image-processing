#include "parent.h"
#include "io_utils.h"
#include "worker.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/select.h>

/* Creates the two pipes for worker slot i and stores each end in the right place.
 * Returns 0 on success, -1 on failure. */
int setup_pipes(worker_info_t *workers, int i, int *child_read_fd, int *child_write_fd) {
    int to_worker[2];
    int from_worker[2];

    if (pipe(to_worker) < 0) {
        perror("pipe to_worker");
        return -1;
    }

    if (pipe(from_worker) < 0) {
        perror("pipe from_worker");
        close(to_worker[0]);
        close(to_worker[1]);
        return -1;
    }

    workers[i].write_fd = to_worker[1];   // parent writes jobs here
    workers[i].read_fd = from_worker[0];  // parent reads results here

    *child_read_fd = to_worker[0];        // child reads jobs from here
    *child_write_fd = from_worker[1];     // child writes results here

    return 0;
}

/* Forks one worker process for slot i. Child closes inherited fds, runs worker_loop(),
 * then exits. Parent closes child-side fds and records the worker in the table.
 * Returns 0 on success, -1 on failure. */
int spawn_worker(worker_info_t *workers, int i, int child_read_fd, int child_write_fd) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        for (int j = 0; j <= i; j++) {
            if (workers[j].write_fd >= 0)
                close(workers[j].write_fd);
            if (workers[j].read_fd >= 0)
                close(workers[j].read_fd);
        }

        worker_loop(i, child_read_fd, child_write_fd);
        _exit(EXIT_FAILURE);
    }

    close(child_read_fd);
    close(child_write_fd);

    workers[i].pid = pid;
    workers[i].alive = 1;
    workers[i].busy = 0;
    workers[i].current_job = -1;

    return 0;
}

/* Blocking read of a worker_hello_msg_t from worker i. Validates msg_type.
 * Returns 0 on success, -1 on read error or unexpected message type. */
int recv_worker_hello(worker_info_t *workers, int i) {
    worker_hello_msg_t hello;

    ssize_t n = read_exact(workers[i].read_fd, &hello, sizeof(hello));

    if (n <= 0) {             
        fprintf(stderr, "parent: failed to read WORKER_HELLO from worker %d\n", i);
        return -1;
    }
    if ((size_t)n != sizeof(hello)) {
        fprintf(stderr, "parent: short read on WORKER_HELLO from worker %d\n", i);
        return -1;
    }
    if (hello.msg_type != MSG_WORKER_HELLO) {
        fprintf(stderr, "parent: expected WORKER_HELLO, got %u from worker %d\n",
                hello.msg_type, i);
        return -1;
    }

    printf("parent: WORKER_HELLO from worker_id=%u pid=%d\n", hello.worker_id, hello.pid);
    return 0;
}

/* Fills a job_msg_t from jobs[job_idx] and writes it to worker i's pipe.
 * Marks the worker busy on success. Returns 0 on success, -1 on failure. */
int send_job_to_worker(worker_info_t *workers, int i, job_t *jobs, int job_idx) {
    job_msg_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.msg_type = MSG_JOB;
    msg.job_id = (uint32_t)jobs[job_idx].job_id;
    msg.filter_type = (uint32_t)jobs[job_idx].filter;

    strncpy(msg.input_path,  jobs[job_idx].input_path, MAX_PATH_LEN - 1);
    msg.input_path[MAX_PATH_LEN - 1] = '\0';

    strncpy(msg.output_path, jobs[job_idx].output_path, MAX_PATH_LEN - 1);
    msg.output_path[MAX_PATH_LEN - 1] = '\0';

    if (write_exact(workers[i].write_fd, &msg, sizeof(msg)) != (ssize_t)sizeof(msg)) {
        perror("parent: write job");
        return -1;
    }

    workers[i].busy = 1;
    workers[i].current_job = job_idx;

    printf("parent: sent job %d (job_id=%d) to worker %d\n", job_idx, jobs[job_idx].job_id, i);
    return 0;
}

/* Blocking read of a result_msg_t from worker i. Prints the result and marks
 * the worker idle. Returns 0 on success, -1 on read error or unexpected type. */
int recv_result_from_worker(worker_info_t *workers, int i) {
    result_msg_t result;

    ssize_t n = read_exact(workers[i].read_fd, &result, sizeof(result));

    if (n <= 0) {
        fprintf(stderr, "parent: failed to read RESULT from worker %d\n", i);
        return -1;
    }
    if ((size_t)n != sizeof(result)) {
        fprintf(stderr, "parent: short read on RESULT from worker %d\n", i);
        return -1;
    }
    if (result.msg_type != MSG_RESULT) {
        fprintf(stderr, "parent: expected MSG_RESULT, got %u from worker %d\n",
                result.msg_type, i);
        return -1;
    }

    printf("parent: RESULT from worker_id=%u job_id=%u status=%u\n", result.worker_id, result.job_id, result.status);

    if (result.status == STATUS_OK) {
        printf("parent: output_path=%s\n", result.output_path);
    } else {
        printf("parent: error_msg=%s\n", result.error_msg);
    }

    workers[i].busy = 0;
    workers[i].current_job = -1;

    return 0;
}

/* Sends a shutdown_msg_t to worker i and closes the write fd.
 * Returns 0 on success, -1 on failure. */
int send_shutdown_to_worker(worker_info_t *workers, int i) {
    shutdown_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.msg_type = MSG_SHUTDOWN;

    if (write_exact(workers[i].write_fd, &msg, sizeof(msg)) != (ssize_t)sizeof(msg)) {
        perror("parent: write shutdown");
        return -1;
    }

    close(workers[i].write_fd);
    workers[i].write_fd = -1;

    printf("parent: sent SHUTDOWN to worker %d\n", i);
    return 0;
}

/* Closes all remaining pipe fds and calls waitpid() on every live worker
 * to reap child processes and prevent zombies. */
void cleanup_workers(worker_info_t *workers, int num_workers) {
    for (int i = 0; i < num_workers; i++) {
        if (workers[i].write_fd >= 0) {
            close(workers[i].write_fd);
            workers[i].write_fd = -1;
        }
        if (workers[i].read_fd >= 0) {
            close(workers[i].read_fd);
            workers[i].read_fd = -1;
        }

        if (workers[i].alive) {
            int status;
            if (waitpid(workers[i].pid, &status, 0) < 0) {
                perror("waitpid");
            } else if (WIFEXITED(status)) {
                printf("parent: worker %d (pid %d) exited with status %d\n", i, workers[i].pid, WEXITSTATUS(status));
            } else {
                printf("parent: worker %d (pid %d) did not exit normally\n", i, workers[i].pid);
            }
            workers[i].alive = 0;
        }
    }
}

/* Top-level orchestrator: spawns workers, waits for hellos, dispatches all jobs,
 * collects results, sends shutdowns, and reaps children. Returns 0 on success. */
int run_parent(int num_workers, job_t *jobs, int num_jobs) {
    worker_info_t workers[MAX_WORKERS];
 
    for (int i = 0; i < num_workers; i++) {
        workers[i].pid = -1;
        workers[i].busy = 0;
        workers[i].alive = 0;
        workers[i].current_job = -1;
        workers[i].write_fd = -1;
        workers[i].read_fd = -1;
    }

    for (int i = 0; i < num_workers; i++) {
        int child_read_fd, child_write_fd;
 
        if (setup_pipes(workers, i, &child_read_fd, &child_write_fd) < 0) {
            fprintf(stderr, "parent: setup_pipes failed for worker %d\n", i);
            cleanup_workers(workers, i);
            return -1;
        }
 
        if (spawn_worker(workers, i, child_read_fd, child_write_fd) < 0) {
            fprintf(stderr, "parent: spawn_worker failed for worker %d\n", i);
            cleanup_workers(workers, i);
            return -1;
        }
    }
 
    for (int i = 0; i < num_workers; i++) {
        if (recv_worker_hello(workers, i) < 0) {
            fprintf(stderr, "parent: hello failed from worker %d\n", i);
            cleanup_workers(workers, num_workers);
            return -1;
        }
    }
 
    int next_job = 0;
    int completed = 0;
 
    for (int i = 0; i < num_workers && next_job < num_jobs; i++) {
        if (send_job_to_worker(workers, i, jobs, next_job) < 0) {
            fprintf(stderr, "parent: send_job failed for worker %d\n", i);
            cleanup_workers(workers, num_workers);
            return -1;
        }
        next_job++;
    }

    while (completed < num_jobs) {
        fd_set read_set;
        FD_ZERO(&read_set);
 
        int max_fd = -1;
 
        for (int i = 0; i < num_workers; i++) {
            if (workers[i].busy) {
                FD_SET(workers[i].read_fd, &read_set);
 
                if (workers[i].read_fd > max_fd) {
                    max_fd = workers[i].read_fd;
                }
            }
        }
 
        int ready = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("select");
            cleanup_workers(workers, num_workers);
            return -1;
        }
 
        for (int i = 0; i < num_workers && completed < num_jobs; i++) {
            if (!workers[i].busy)
                continue;
 
            if (!FD_ISSET(workers[i].read_fd, &read_set))
                continue;

            if (recv_result_from_worker(workers, i) < 0) {
                fprintf(stderr, "parent: recv_result failed for worker %d\n", i);
                cleanup_workers(workers, num_workers);
                return -1;
            }
            completed++;
 
            if (next_job < num_jobs) {
                if (send_job_to_worker(workers, i, jobs, next_job) < 0) {
                    fprintf(stderr, "parent: send_job failed for worker %d\n", i);
                    cleanup_workers(workers, num_workers);
                    return -1;
                }
                next_job++;
            }
        }
    }
 
    for (int i = 0; i < num_workers; i++) {
        if (send_shutdown_to_worker(workers, i) < 0) {
            fprintf(stderr, "parent: shutdown failed for worker %d\n", i);
        }
    }
 
    cleanup_workers(workers, num_workers);
 
    printf("parent: all jobs done, all workers exited\n");
    return 0;
}