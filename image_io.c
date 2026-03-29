#include "image_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int read_ppm(const char *path, image_t *img) {
    // Open file
    FILE *img_fp = fopen(path, "rb");

    if (!img_fp) {
        perror("Error opening file");
        return -1;
    }
    img->data = NULL;

    // Read magic number
    int char1 = fgetc(img_fp);
    int char2 = fgetc(img_fp);

    if (char1 != 'P' || char2 != '6') {
        fprintf(stderr, "Error: File from %s is not a P6 PPM file\nExpected magic number: P6\nActual magic number: %c%c\n", path, char1, char2);
        fclose(img_fp);
        return -1;
    }

    // Skip whitespace and comments
    char line[1024];
    int line_char;

    while ((line_char = getc(img_fp)) != EOF) {
        if (line_char == '#') {
            while ((line_char = getc(img_fp)) != EOF && line_char != '\n');
        } else if (!isspace((unsigned char)line_char)) {
            ungetc(line_char, img_fp);
            break;
        }
    }

    // Read width, height, maxval
    int width; 
    int height;
    int maxval;

    if (fscanf(img_fp, "%d %d %d", &width, &height, &maxval) != 3) {
        fprintf(stderr, "Error: Invalid PPM headers in image file from %s\n", path);
        fclose(img_fp);
        return -1;
    }

    if (maxval != 255) {
        fprintf(stderr, "Error: Image file maxval is not 255 in image file from %s\nExpected maxval: 255\nActual maxval: %d\n", path, maxval);
        fclose(img_fp);
        return -1;
    }

    img->width = width;
    img->height = height;

    // Consume single whitespace separator
    fgetc(img_fp); 

    // Allocate pixel buffer
    size_t total_bytes = (size_t)img->width * (size_t)img->height * 3;
    img->data = malloc(total_bytes);

    if (img->data == NULL) {
        fprintf(stderr, "Error: Memory allocation fails for image file from %s\n", path);
        fclose(img_fp);
        return -1;
    }

    // Read binary pixel data
    size_t bytes_read = fread(img->data, 1, total_bytes, img_fp);
    if (bytes_read != total_bytes) {
        fprintf(stderr, "Error: Short read in image file from %s\nExpected bytes read: %zu\nActual bytes read: %zu\n", path, total_bytes, bytes_read);
        fclose(img_fp);
        free_image(img);
        return -1;
    }
    
    // Success
    fclose(img_fp);
    return 0;
}

int write_ppm(const char *path, const image_t *img) {
    // Open file
    FILE *img_fp = fopen(path, "wb");
    if (!img_fp) {
        perror("Error opening file");
        return -1;
    }

    // Write P6 (binary) PPM file header
    fprintf(img_fp, "P6\n%d %d\n255\n", img->width, img->height);

    // Write binary pixel data
    size_t total_bytes = (size_t)img->width * (size_t)img->height * 3;
    size_t bytes_written = fwrite(img->data, 1, total_bytes, img_fp);
    if (bytes_written != total_bytes) {
        fprintf(stderr, "Error: Short write in image file from %s\nExpected bytes written: %zu\nActual bytes written: %zu\n", path, total_bytes, bytes_written);
        fclose(img_fp);
        return -1;
    }
    
    // Success
    fclose(img_fp);
    return 0;
}

void free_image(image_t *img) {
    if (img == NULL) {
        return;
    }

    if (img->data != NULL) {
        free(img->data);
        img->data = NULL;
    }
}

int read_image(const char *path, file_type_t type, image_t *img) {
    switch (type) {
        case FILE_PPM:
            return read_ppm(path, img);
        default:
            fprintf(stderr, "Error: unsupported file type\n");
            return -1;
    }
}

int write_image(const char *path, file_type_t type, const image_t *img) {
    switch (type) {
        case FILE_PPM:
            return write_ppm(path, img);
        default:
            fprintf(stderr, "Error: unsupported file type\n");
            return -1;
    }
}