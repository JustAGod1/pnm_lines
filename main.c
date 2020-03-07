#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

float gamma_correction(float x) {
    if (x < 0) x = 0;
    if (x > 1) x = 1;
    float r;

    if (x <= 0.03928) {
        r = x / 12.92;
    } else {
        r = pow((0.055+x)/1.055, 2.4);
    }
    if (r > 1) return 1;
    else if (r < 0) return 0;

    return r;
}

void apply_pixel(unsigned char **bitmap, int x, int y, float alpha, unsigned char baseColor) {
    if (x < 0 || y < 0)
        return;
    if (alpha > 1) alpha = 1;
    bitmap[y][x] = (char)(((float)((1 - alpha) * bitmap[y][x] + gamma_correction(alpha * baseColor / 255.0) * 255.0)));

}



int integer_part(float a) {
    return (int) (a);
}

int ceil_floor_number(float a) {
    return integer_part(a + 0.5);
}

float fraction_part(float a) {
    if (a < 0) {
        return a - (integer_part(a) + 1);
    } else {
        return a - integer_part(a);
    }
}

void swap_number(int *a, int *b) {
    int t = *a;
    *a = *b;
    *b = t;
}


float absolute_number(float a) {
    if (a < 0) {
        return -a;
    } else {
        return a;
    }
}

float return_fraction_part(float a) {
    return 1 - fraction_part(a);
}

void draw_wu_line(unsigned char **matrix, int x0, int y0, int x1, int y1, unsigned char color) {
    int steep = absolute_number(y1 - y0) > absolute_number(x1 - x0);
    if (steep) {
        swap_number(&x0, &y0);
        swap_number(&x1, &y1);
    }
    if (x0 > x1) {
        swap_number(&x0, &x1);
        swap_number(&y0, &y1);
    }

    if (!steep) {
        apply_pixel(matrix, x0, y0, 1, color);
        apply_pixel(matrix, x1, y1, 1, color);
    } else {
        apply_pixel(matrix, y0, x0, 1, color);
        apply_pixel(matrix, y1, x1, 1, color);
    }
    float dx = x1 - x0;
    float dy = y1 - y0;
    float gradient = dy / dx;
    float y = y0 + gradient;
    for (int x = x0 + 1; x <= x1 - 1; x++) {
        if (!steep) {
            apply_pixel(matrix, x, (int) y, 1 - (y - (int) y), color);
            apply_pixel(matrix, x, (int) y + 1, y - (int) y, color);
        } else {
            apply_pixel(matrix, (int) y, x, 1 - (y - (int) y), color);
            apply_pixel(matrix, (int) y + 1, x, y - (int) y, color);

        }
        y += gradient;
    }
}

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
        int x1, int y1,
        int x2, int y2,
        unsigned char **matrix, unsigned width, unsigned height,
        int line_width, int line_color) {
    double a = ((float) (y2 - y1)) / ((float) (x2 - x1));
    double b = y1 - a * x1;
    double line_width_sq = (line_width - 1) * (line_width - 1) / 4.0;
    double ar = -(1 / a);

    if (x1 < 0 || y1 < 0 || x2 >= width || y2 >= height) {
        printf("Endpoints exceeds image size");
        exit(1);
    }

    if (line_width > 1) {
        draw_wu_line(matrix, x1 - line_width / 2, y1, x2 - line_width / 2, y2, line_color);
        draw_wu_line(matrix, x1 + line_width / 2, y1, x2 + line_width / 2, y2, line_color);
    } else {
        draw_wu_line(matrix, x1, y1, x2, y2, line_color);
    }
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

int satoi(char *input) {
    char *err = 0;
    long int result = strtol(input, &err, 10);
    if (!err || err[0] == 0 || result != 0) return result;
    printf("Cannot parse %s to int\n", input);
    exit(1);
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

    unsigned char **matrix;
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


    FILE *output = fopen(args[2], "wb");
    if (output == NULL) {
        printf("Error %d while opening file %s", errno, args[2]);
        return 1;
    }
    fprintf(output, "P5\n%d %d\n%d\n", width, height, 255);

    write_image(
            output,
            satoi(args[5]), satoi(args[6]),
            satoi(args[7]), satoi(args[8]) ,
            matrix,
            width, height,
            satoi(args[4]), satoi(args[3])
    );

    for (int j = 0; j < height; ++j) {
        free(matrix[j]);
    }
    free(matrix);


    fclose(output);

    return 0;
}