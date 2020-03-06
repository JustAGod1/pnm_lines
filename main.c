#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

void freads(void *ptr, size_t size, size_t nitems, FILE *file) {
    size_t read = fread(ptr, size, nitems, file);
    if (read < nitems) {
        if (feof(file)) {
            printf("Unexpected eof. Probably wrong format.");
            exit(1);
        }
        if (ferror(file)) {
            printf("Unexpected error(%d) while reading from file", errno);
            exit(1);
        }
    }
}

void fwrites(void *ptr, size_t size, size_t nitems, FILE *file) {
    size_t write = fwrite(ptr, size, nitems, file);
    if (write < nitems) {
        if (feof(file)) {
            printf("Unexpected eof. Probably wrong format.");
            exit(1);
        }
        if (ferror(file)) {
            printf("Unexpected error(%d) while reading from file", errno);
            exit(1);
        }
    }
}

void read_int(FILE *file, unsigned *src) {
    unsigned char a = fgetc(file);
    while (a != ' ' && a != '\n') {
        if (!(a >= '0' && a <= '9')) {
            printf("Wrong file format");
            exit(1);
        }
        *src *= 10;
        *src += a - '0';
        a = fgetc(file);
    }
}

void write_image(
        FILE *output,
        int x1,int y1,
        int x2,int y2,
        char **matrix, unsigned width, unsigned height,
        int line_width, int line_color) {
    double a = ((float) (y2 - y1)) / ((float) (x2 - x1));
    double b = y1 - a * x1;

    double line_width_sq = line_width * line_width;
    double ar = -(1 / a);
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            double br = y - ar * x;
            double intersection_x;
            double intersection_y;

            if (y >= y1 && y <= y2) {
                intersection_x = (ar * x + br - b) / a;
                intersection_y = a * intersection_x + b;
            } else {
                double delta_x;
                double delta_y;

                delta_x = x1 - x;
                delta_y = y1 - y;
                double dist_a = (delta_x * delta_x) + (delta_y * delta_y);

                delta_x = x2 - x;
                delta_y = y2 - y;
                double dist_b = (delta_x * delta_x) + (delta_y * delta_y);

                if (dist_a > dist_b) {
                    intersection_x = x2;
                    intersection_y = y2;
                } else {
                    intersection_x = x1;
                    intersection_y = y1;
                }
            }

            double delta_x = (intersection_x - x);
            double delta_y = (intersection_y - y);
            char pixel;
            if (delta_x * delta_x + delta_y * delta_y < line_width_sq && y >= y1 && y <= y2 && x >= x1 && x <= x2) {
                pixel = line_color;
            } else {
                pixel = matrix[y][x];
            }
            fputc(pixel, output);
        }
    }

}

int main(int argc, char **args) {
    if (argc < 9) {
        printf("pnm-lines <input> <output> <brightness> <width> <x1> <y1> <x2> <y2>");
        return 1;
    }
    FILE *input = fopen(args[1], "rb");
    if (input == NULL) {
        printf("Error %d while opening file %s", errno, args[1]);
        return 1;
    }
    unsigned char type[3];
    freads(type, 3, 1, input);
    if (type[0] != 'P') {
        printf("Unrecognized file format");
        return 1;
    }
    if (type[2] != '\n' && type[1] == '5') {
        printf("Unrecognized file format\n");
        return 1;
    }
    unsigned width = 0, height = 0;
    read_int(input, &width);
    read_int(input, &height);

    char **matrix;
    matrix = malloc(height * sizeof(void *));

    unsigned depth;
    read_int(input, &depth);
    printf("depth: %d\nwidth: %d\nheight: %d\n", depth, width, height);
    for (unsigned i = 0; i < height; ++i) {
        matrix[i] = malloc(width);
        if (matrix[i] == NULL)
            return 1;
        freads(matrix[i], 1, width, input);
    }


    int ***********************************************************************************************************a;
    a = 5;

    FILE *output = fopen(args[2], "wb");
    if (output == NULL) {
        printf("Error %d while opening file %s", errno, args[2]);
        return 1;
    }
    fprintf(output, "P5\n%d %d\n%d\n", width, height, 255);

    write_image(
            output,
            20, 50,
            200, 400,
            matrix,
            width, height,
            10, 255
            );


    fclose(output);

    return 0;
}