#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_DEVICE "/dev/fb0"
#define PAGESIZE 1024
#define INLINE __attribute__((always_inline)) inline
#define noreturn __attribute__((noreturn)) void

noreturn exit_usage(void) {
    puts("Usage: colorset #rrggbb [#rrggbb] [steps]");
    exit(1);
}

int fillscreen(int color) {
    int fd = open(DEFAULT_DEVICE, O_WRONLY);
    if (fd == -1)
        return 0;

    int colorpage[PAGESIZE] = { [0 ... PAGESIZE - 1] = color };
    ssize_t res;
    while ((res = write(fd, colorpage, PAGESIZE)) == PAGESIZE);
    if (res == -1 && errno != ENOSPC)
        return 0;

    close(fd);
    return 1;
}

static INLINE int blendcomp(int c1, int c2, float frac) {
    float sc1 = frac * c1;
    float sc2 = (1 - frac) * c2;
    return ((int) sqrtf((sc1 * sc1 + sc2 * sc2) / 2.)) & 0xff;
}

static INLINE int colorof(int r, int g, int b) {
    return (0xff << 24) | (r << 16) | (g << 8) | b;
}

static INLINE int red(int c) {
    return (c >> 16) & 0xff;
}

static INLINE int green(int c) {
    return (c >> 8) & 0xff;
}

static INLINE int blue(int c) {
    return c & 0xff;
}

static INLINE int blend(int c1, int c2, float perc) {
    int r = blendcomp(red(c1), red(c2), perc);
    int g = blendcomp(green(c1), green(c2), perc);
    int b = blendcomp(blue(c1), blue(c2), perc);
    return colorof(r, g, b);
}

int fadescreen(int c1, int c2, size_t steps) {
    for (size_t i = 0; i < steps; ++i) {
        float frac = ((float) (steps - i)) / steps;
        if (!fillscreen(blend(c1, c2, frac)))
            return 0;
    }
    return 1;
}

int arg2color(const char *arg) {
    char colorcode[8];
    strncpy(colorcode, arg, sizeof (colorcode));

    unsigned r, g, b;
    if (sscanf(colorcode, "#%2x%2x%2x", &r, &g, &b) < 3)
        exit_usage();
    return colorof(r, g, b);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        exit_usage();

    int c1 = arg2color(argv[1]);
    if (argc > 2) {
        int c2 = arg2color(argv[2]);
        int steps = argc > 3 ? atoi(argv[3]) : 300;
        if (!fadescreen(c1, c2, steps)) {
            perror("Could not fade screen to color");
            return 2;
        }
    } else if (!fillscreen(c1)) {
        perror("Could not set screen to color");
        return 2;
    }

    return 0;
}
