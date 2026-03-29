#ifndef JOBS_H
#define JOBS_H

#include "protocol.h"

/*
 * Represents a single image processing job to be executed by a worker.
 *
 * job_id      - unique identifier for this job; used by the parent
 *               to match results back to the original request
 * filter      - choice of filter the worker should apply
 *               (FILTER_GREY, FILTER_BLUR, or FILTER_EDGE)
 * input_path  - null-terminated file system path to the source PPM image file
 * output_path - null-terminated file system path where the processed image will
 *               be written; the worker creates or overwrites this file
 */
typedef struct {
    int job_id;
    filter_t filter;
    char input_path[MAX_PATH_LEN];
    char output_path[MAX_PATH_LEN];
} job_t;

#endif