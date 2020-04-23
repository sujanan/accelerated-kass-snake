#ifndef SNAKE_H
#define SNAKE_H

#ifdef __cplusplus
  #define EXTERNC extern "C"
#else
  #define EXTERNC
#endif

// ========================
// make snake.cpp stanalone
// #define SNAKE_STANDALONE
// ========================

struct image;

EXTERNC struct image *imageNew();
EXTERNC void imageInit(struct image *im);
EXTERNC void imageRead(struct image *im, const char *filename);
EXTERNC unsigned char *imageGetData(struct image *im);
EXTERNC int imageWidth(struct image *im);
EXTERNC int imageHeight(struct image *im);
EXTERNC void imageFree(struct image *im);

struct contour;

EXTERNC struct contour *contourNew();
EXTERNC void contourInit(struct contour *con, int size);
EXTERNC void contourPush(struct contour *con, double x, double y);
EXTERNC int contourSize(struct contour *con);
EXTERNC void contourFree(struct contour *con);

struct energy;

EXTERNC struct energy *energyNew();
EXTERNC void energyInit(struct energy *en);
EXTERNC void energyCalculateForce(
        struct energy *enptr, 
        struct image *imptr, 
        double sigma);

struct snake;

EXTERNC struct snake *snakeNew();
EXTERNC void snakeInit(
        struct snake *snake, 
        struct image *im, 
        struct contour *con, 
        struct energy *en,
        double alpha,
        double beta,
        double gamma);
EXTERNC void snakeExec(struct snake *snake, int niter);
EXTERNC void snakeFree(struct snake *snake);

#endif
