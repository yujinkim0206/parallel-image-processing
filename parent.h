#ifndef PARENT_H
#define PARENT_H

#include <sys/types.h>
#include "protocol.h"
#include "jobs.h"

#define MAX_WORKERS 64

typedef struct {
    pid_t pid;
    int busy;         // 1 = busy, 0 = idle
    int alive;        // 1 = alive, 0 = dead
    int current_job;  // index into jobs[], -1 if none
    int write_fd;     // parent -> worker pipe (write end)
    int read_fd;      // worker -> parent pipe (read end)
} worker_info_t;

int run_parent(int num_workers, job_t *jobs, int num_jobs);

/* helpers */
int setup_pipes(worker_info_t *workers, int i, int *child_read_fd, int *child_write_fd);
int spawn_worker(worker_info_t *workers, int i, int child_read_fd, int child_write_fd, int num_workers);
int recv_worker_hello(worker_info_t *workers, int i);
int send_job_to_worker(worker_info_t *workers, int i, job_t *jobs, int job_idx);
int recv_result_from_worker(worker_info_t *workers, int i);
int send_shutdown_to_worker(worker_info_t *workers, int i);
void cleanup_workers(worker_info_t *workers, int num_workers);

#endif