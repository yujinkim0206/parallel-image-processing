#ifndef WORKER_H
#define WORKER_H

/*
 * Worker entry point.
 *
 * worker_id - logical worker ID
 * read_fd   - pipe read end (parent -> worker)
 * write_fd  - pipe write end (worker -> parent)
 *
 * This function does not return under normal operation.
 */
void worker_loop(int worker_id, int read_fd, int write_fd);

#endif /* WORKER_H */