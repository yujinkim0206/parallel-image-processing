#include "filters.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Clamp helper function to clamp an integer to the range [0, 255].
 */
static unsigned char clamp(int value) {
    if (value < 0) {
        return 0;
    } else if (value > 255) {
        return 255;
    }   
    return value;
}

int apply_greyscale(image_t *img) {
    if (img == NULL || img->data == NULL) {
        return -1;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            // Get original color values
            unsigned char R = PIXEL_R(img, x, y);
            unsigned char G = PIXEL_G(img, x, y);
            unsigned char B = PIXEL_B(img, x, y);

            // Compute luminance using ITU-R BT.601 formula
            // L = 0.299 * R + 0.587 * G + 0.114 * B
            unsigned char L = (unsigned char)(0.299 * R + 0.587 * G + 0.114 * B);

            // Set all channels to same luminance value in-place
            PIXEL_R(img, x, y) = L;
            PIXEL_G(img, x, y) = L;
            PIXEL_B(img, x, y) = L;
        }
    }

    
    return 0;
}

int apply_blur(image_t *img) {
    if (img == NULL || img->data == NULL) {
        return -1;
    }

    // Allocate a temporary buffer of the same size as img->data
    size_t total_bytes = (size_t)img->width * (size_t)img->height * 3;
    unsigned char *temp_img_data = malloc(total_bytes);

    if (temp_img_data == NULL) {
        perror("apply_blur: malloc failed");
        return -1;
    }

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int R_SUM = 0;
            int G_SUM = 0;
            int B_SUM = 0;
            int count = 0;

            // Loop over 3x3 neighbourhood for every pixel
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int cur_x = x + dx;
                    int cur_y = y + dy;

                    // Sum each channel
                    if (cur_x >= 0 && cur_x < img->width && cur_y >= 0 && cur_y < img->height) {
                        size_t index = ((size_t)cur_y * img->width + cur_x) * 3;
                        R_SUM += img->data[index];
                        G_SUM += img->data[index + 1];
                        B_SUM += img->data[index + 2];
                        count++;
                    }
                }
            }

            // Divide channel sum by count and write averaged values into temporary buffer
            size_t target_index = ((size_t)y * img->width + x) * 3;
            temp_img_data[target_index] = (unsigned char)(R_SUM / count);
            temp_img_data[target_index + 1] = (unsigned char)(G_SUM / count);
            temp_img_data[target_index + 2] = (unsigned char)(B_SUM / count);
        }
    }

    // Copy temporary buffer back into img->data
    memcpy(img->data, temp_img_data, total_bytes);

    // Free temporary buffer
    free(temp_img_data);

    return 0;
}

int apply_edge_detect(image_t *img) {
    if (img == NULL || img->data == NULL) {
        return -1;
    }
    
    // Allocate a temporary buffer of the same size as img->data
    size_t total_bytes = (size_t)img->width * (size_t)img->height * 3;
    unsigned char *temp_img_data = malloc(total_bytes);

    if (temp_img_data == NULL) {
        perror("apply_edge_detect: malloc failed");
        return -1;
    }

    int Gx_kernel[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy_kernel[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int Gx = 0;
            int Gy = 0;

            // Loop over 3x3 neighbourhood for every pixel
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int cur_x = x + dx;
                    int cur_y = y + dy;

                    // Greyscale neighbourhood
                    // Compute luminance using ITU-R BT.601 formula
                    // L = 0.299 * R + 0.587 * G + 0.114 * B
                    if (cur_x >= 0 && cur_x < img->width && cur_y >= 0 && cur_y < img->height) {
                        size_t index = ((size_t)cur_y * img->width + cur_x) * 3;
                        unsigned char grey = (unsigned char)(0.299 * img->data[index] + 0.587 * img->data[index + 1] + 0.114 * img->data[index + 2]);
                        Gx += grey * Gx_kernel[dy + 1][dx + 1];
                        Gy += grey * Gy_kernel[dy + 1][dx + 1];
                    }
                }
            }

            // Calculate magnitude
            int magnitude = clamp((int)sqrt((double)(Gx * Gx + Gy * Gy)));
            size_t target_index = ((size_t)y * img->width + x) * 3;
            temp_img_data[target_index] = (unsigned char)(magnitude);
            temp_img_data[target_index + 1] = (unsigned char)(magnitude);
            temp_img_data[target_index + 2] = (unsigned char)(magnitude);
        }
    }

    // Copy temporary buffer back into img->data
    memcpy(img->data, temp_img_data, total_bytes);

    // Free temporary buffer
    free(temp_img_data);

    return 0;
}

int apply_filter(image_t *img, filter_t filter) {
    switch (filter) {
        case FILTER_GREY: 
            return apply_greyscale(img);
        case FILTER_BLUR:
            return apply_blur(img);
        case FILTER_EDGE:
            return apply_edge_detect(img);
        default:
            fprintf(stderr, "apply_filter: unrecognized filter type %d\n", (int)filter);
            return -1;
    }
}