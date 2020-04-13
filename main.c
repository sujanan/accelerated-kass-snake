#include <stdio.h>
#include <GL/glut.h>
#include "snake.h"

// OpenGL setup
// ========================================================
struct graphicsUtilConfig {
    int win_width;
    int win_height;
    char *win_name;
    int display_mode;
} static const graphics_util_config_default = {
    480,
    640,
    "Kass snake",
    GLUT_DOUBLE | GLUT_RGB,
};

void openWindow(
        struct graphicsUtilConfig *config,
        int *argc,
        char **argv) {

    glutInit(argc, argv);
    glutInitDisplayMode(config->display_mode);
    glutCreateWindow(config->win_name);
    glutMainLoop();
}

#ifndef SNAKE_STANDALONE
int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s [image]\n", argv[0]);
        return 1; 
    }
    const char *filename = argv[1];
    struct image *im = imageNew();
    imageInit(im);
    imageRead(im, filename);
    imageFree(im);
    struct graphicsUtilConfig config = graphics_util_config_default;
    openWindow(&config, &argc, argv);
    return 0;
}
#endif

