#ifndef FILTERS_H
#define FILTERS_H

#include "image_io.h"
#include "protocol.h"

/*
 * Apply the specified filter to img in-place.
 * This is the main entry point called by the worker.
 *
 * img    - the image to process specified filter; modified in-place
 * filter - which filter to apply (FILTER_GREY, FILTER_BLUR, FILTER_EDGE)
 *
 * Return value:
 *    0    : success
 *    -1   : error (unrecognized filter, or internal error)
 */
int apply_filter(image_t *img, filter_t filter);

/*
 * Convert img to greyscale in-place using ITU-R BT.601 luminance weights:
 *   L = 0.299 * R + 0.587 * G + 0.114 * B
 * All three channels of each pixel are set to L so the image
 * remains a valid RGB PPM file.
 *
 * img - the image to convert to greyscale; modified in-place
 *
 * Return value:
 *    0    : success
 *    -1   : error (img == NULL or img->data == NULL)
 */
int apply_greyscale(image_t *img);

/*
 * Apply a 3x3 box blur to img in-place.
 * Each output pixel is the average of itself and its neighbours.
 * Border pixels use only the neighbours that exist within the image.
 *
 * img - the image to blur; modified in-place
 * 
 * Return value:
 *    0    : success
 *    -1   : error (img == NULL, img->data == NULL, or memory allocation for temporary buffer fails)
 */
int apply_blur(image_t *img);

/*
 * Apply Sobel edge detection to img in-place.
 * Each output pixel represents the gradient magnitude at that location.
 * Strong edges appear bright; smooth regions appear dark.
 * The output is a greyscale edge map stored as an RGB PPM.
 *
 * img - the image to process edge detection; modified in-place
 *
 * Return value:
 *    0    : success
 *    -1   : error (img == NULL, img->data == NULL, or memory allocation for temporary buffer fails)
 */
int apply_edge_detect(image_t *img);

#endif