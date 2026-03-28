#ifndef IMAGE_IO_H
#define IMAGE_IO_H

#include "protocol.h"

#define PIXEL_R(img, x, y) ((img)->data[(((y) * (img)->width + (x)) * 3) + 0])
#define PIXEL_G(img, x, y) ((img)->data[(((y) * (img)->width + (x)) * 3) + 1])
#define PIXEL_B(img, x, y) ((img)->data[(((y) * (img)->width + (x)) * 3) + 2])

/*
 * Represents a PPM image loaded into memory.
 *
 * width  - number of pixel columns
 * height - number of pixel rows
 * data   - flat array (or pointer) containing RGB values
 *          total byte size is (width * height * 3) bytes
 *          for pixel (0,0), the first byte is R, second is G, third is B,
 *          then for pixel (1,0), the first byte is R, second is G, third is B, 
 *          and so on across each row
 */
typedef struct {
    int width;
    int height;
    unsigned char *data;
} image_t;

/*
 * Reads a PPM file from disk into img.
 * Only P6 (binary) PPM files with maxval 255 are supported.
 *
 * path - null-terminated file system path where the input .ppm file is located
 * img  - pointer to the struct that will store the loaded image data
 * 
 * Return value:
 *    0    : success, img->data is allocated on the heap and must be released using free_image() to avoid memory leaks
 *    -1   : error (the file cannot be opened, the header is wrong, or incomplete pixel data )
 */
int read_ppm(const char *path, image_t *img);

/*
 * Writes img to disk as a P6 (binary) PPM file.
 *
 * path - null-terminated file system path to save the output .ppm file
 * img  - the image to save; must have valid width, height, and data
 *
 * Return value:
 *    0    : success
 *    -1   : error (the file cannot be created, or something went wrong while writing)
 */
int write_ppm(const char *path, const image_t *img);

/*
 * Frees the pixel data inside img and sets img->data to NULL.
 * Safe to call even if img or img->data is already NULL.
 */
void free_image(image_t *img);

/*
 * A wrapper around read_ppm that checks the file type first.
 * Currently only FILE_PPM is supported.
 *
 * path - null-terminated file system path where the input image file is located
 * type - the format of the file (currently only FILE_PPM is supported)
 * img  - gets passed through to read_ppm
 *
 * Return value:
 *    0    : success
 *    -1   : error (unsupported file, or read_ppm fails)
 */
int read_image(const char *path, file_type_t type, image_t *img);

/*
 * A wrapper around write_ppm that checks the file type first.
 * Currently only FILE_PPM is supported.
 *
 * path - null-terminated file system path to save the output image file
 * type - the format to write (currently only FILE_PPM is supported)
 * img  - the image to save; passed through to write_ppm
 *
 * Return value:
 *    0    : success
 *    -1   : error (unsupported file, or write_ppm fails)
 */
int write_image(const char *path, file_type_t type, const image_t *img);

#endif /*IMAGE_IO_H*/