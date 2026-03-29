#include "jobs.h"
#include "image_io.h"
#include "filters.h"

#include <stddef.h>

int run_job(const job_t *job) {
    if (job == NULL) {
        return -1;
    }

    // Initialize struct for img
    image_t img;
    img.width = 0;
    img.height = 0;
    img.data = NULL;

    // Step 1: Read a PPM file from disk into img
    if (read_ppm(job->input_path, &img) != 0) {
        return -1;
    }

    // Step 2: Apply requested filter to img in-place
    if (apply_filter(&img, job->filter) != 0) {
        free_image(&img);
        return -1;
    }

    // Step 3: Write the result img to disk as a P6 (binary) PPM file
    if (write_ppm(job->output_path, &img) != 0) {
        free_image(&img);
        return -1;
    }

    // Step 4: Success; free pixel buffer
    free_image(&img);

    // Success
    return 0;
}