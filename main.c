#include "protocol.h"
#include "parent.h"
#include "jobs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_WORKERS 4

/* Converts filter string ("grey", "blur", "edge") to filter_t enum.
 * Returns 0 on success, -1 if the string is invalid. */
static int parse_filter(const char *filter_str, filter_t *filter)
{
    if (strcmp(filter_str, "grey") == 0) {
        *filter = FILTER_GREY;
        return 0;
    }
    if (strcmp(filter_str, "blur") == 0) {
        *filter = FILTER_BLUR;
        return 0;
    }
    if (strcmp(filter_str, "edge") == 0) {
        *filter = FILTER_EDGE;
        return 0;
    }
    return -1;
}

/* Parses CLI arguments into job_t array and hands off to
 * run_parent() which handles worker spawning, scheduling, and cleanup. */
int main(int argc, char *argv[])
{
    /* each job = input output filter  → 3 args */
    if (argc < 4 || ((argc - 1) % 3 != 0)) {
        fprintf(stderr,
                "Usage: %s <input1> <output1> <grey|blur|edge> "
                "[<input2> <output2> <grey|blur|edge> ...]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    int num_workers = DEFAULT_WORKERS;
    int num_jobs = (argc - 1) / 3;

    job_t *jobs = malloc((size_t)num_jobs * sizeof(job_t));
    if (!jobs) {
        perror("malloc jobs");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_jobs; i++) {
        const char *input_path  = argv[1 + 3 * i];
        const char *output_path = argv[2 + 3 * i];
        const char *filter_str  = argv[3 + 3 * i];

        filter_t filter;
        if (parse_filter(filter_str, &filter) != 0) {
            fprintf(stderr, "Invalid filter: %s\n", filter_str);
            free(jobs);
            return EXIT_FAILURE;
        }

        memset(&jobs[i], 0, sizeof(job_t));

        jobs[i].job_id = i + 1;
        jobs[i].filter = filter;
        jobs[i].file_type = FILE_PPM;

        strncpy(jobs[i].input_path, input_path, MAX_PATH_LEN - 1);
        strncpy(jobs[i].output_path, output_path, MAX_PATH_LEN - 1);
    }

    printf("parent: starting %d workers for %d jobs\n",
           num_workers, num_jobs);

    int ret = run_parent(num_workers, jobs, num_jobs);

    free(jobs);
    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}