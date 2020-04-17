#include <cmath>
#include <iostream>
#include <diplib.h>
#include <diplib/linear.h>
#include <diplib/analysis.h>
#include <diplib/simple_file_io.h>
#include <xtensor/xarray.hpp>
#include <xtensor-blas/xlinalg.hpp>
#include "snake.h"

#define SNAKE_DEBUG 1

// Image API
// ========================================================
struct image {
    dip::Image dip_img;
};

EXTERNC struct image *imageNew() {
    return new image();
}

EXTERNC void imageInit(struct image *im) {
    im->dip_img = dip::Image();
}

EXTERNC void imageRead(struct image *im, const char *filename) {
    dip::ImageRead(im->dip_img, std::string(filename));
}

EXTERNC unsigned char *imageGetData(struct image *im) {
    return (unsigned char *) im->dip_img.Data();
}

EXTERNC int imageWidth(struct image *im) {
    return im->dip_img.Sizes()[0];
}

EXTERNC int imageHeight(struct image *im) {
    return im->dip_img.Sizes()[1];
}

EXTERNC void imageFree(struct image *im) {
    delete im;
}

// Contour creation/manipulation API
// ========================================================
struct contour {
    std::vector<double> x;
    std::vector<double> y;
};

EXTERNC struct contour *contourNew() {
    return new contour();
}

EXTERNC void contourInit(struct contour *con, int size = 0) {
    // Since our contour is strict let's
    // do nothing here. When contour becomes
    // more flexible we'll have a correct size.
}

EXTERNC void contourCreate(struct contour *con) {
    con->x.resize(63);
    con->y.resize(63);
    int i = 0;
    for (double v = 0.0; v < 2 * M_PI; v += 0.1)
        con->x[i++] = 120 + 50 * cos(v);
    i = 0;
    for (double v = 0.0; v < 2 * M_PI; v += 0.1)
        con->y[i++] = 140 + 60 * sin(v);
}

EXTERNC int contourSize(struct contour *con) {
    return con->x.size();
}

EXTERNC void contourFree(struct contour *con) {
    delete con;
}

// Energy API
// ========================================================
struct energy {
    dip::Image dip_force; 
};

EXTERNC struct energy *energyNew() {
    return new energy();
}

EXTERNC void energyInit(struct energy *en) {
    en->dip_force = dip::Image();
}

EXTERNC void energyCalculateForce(
        struct energy *en, 
        struct image *im, 
        double sigma) {
    dip::GradientMagnitude(im->dip_img, en->dip_force, { sigma });
    dip::Gradient(en->dip_force, en->dip_force);
}

EXTERNC void energyFree(struct energy *en) {
    delete en;
}

// Utils
// ========================================================
#ifdef SNAKE_DEBUG
void utilShowImagePixels(const dip::Image& im) {
    if (im.Sizes().size() == 0)
        return;
    int len = im.Sizes()[0];
    for (int i = 1; i < im.Sizes().size(); i++)
        len *= im.Sizes()[i];
    for (int i = 0; i < len; i++)
        std::cout << im.At(i) << std::endl;
}
#endif

// Snake API
// ========================================================
struct snake {
    std::vector<double> mat;
    struct image im;
    struct contour con;
    struct energy exteng;
    double alpha;
    double beta;
    double gamma;
};

EXTERNC struct snake *snakeNew() {
    return new snake();
}

static void fillMatrixSnake(struct snake& snake) {
    double a = snake.gamma * (2 * snake.alpha + 6 * snake.beta) + 1;
    double b = snake.gamma * (-snake.alpha - 4 * snake.beta);
    double c = snake.gamma * snake.beta;
    int n = contourSize(&snake.con);
    for (int i = 0; i < n; i++) {
        int in2 = ((i - 2) < 0) ? n + (i - 2) : (i - 2);
        int in1 = ((i - 1) < 0) ? n + (i - 1) : (i - 1);
        int ip1 = (i + 1) % n;
        int ip2 = (i + 2) % n;
        for (int j = 0; j < n; j++) {
            if (in2 == j)
                snake.mat[i * n + j] = c;
            else if (in1 == j)
                snake.mat[i * n + j] = b;
            else if (i == j)
                snake.mat[i * n + j] = a;
            else if (ip1 == j)
                snake.mat[i * n + j] = b;
            else if (ip2 == j)
                snake.mat[i * n + j] = c;
            else
                snake.mat[i * n + j] = 0.0;
        }
    }
    std::vector<size_t> shape = { (size_t) n, (size_t) n };
    xt::xarray<double, xt::layout_type::dynamic> mat(shape, 
            xt::layout_type::row_major);
    for (int i = 0; i < n * n; i++)
        mat.data()[i] = snake.mat[i];
    mat = xt::linalg::inv(mat);
    for (int i = 0; i < n * n; i++)
        snake.mat[i] = mat.data()[i];
}

EXTERNC void snakeInit(
        struct snake *snake, 
        struct image *im, 
        struct contour *con, 
        struct energy *en,
        double alpha,
        double beta,
        double gamma) {
    snake->im = *im;
    snake->con = *con;
    snake->exteng = *en;
    snake->alpha = alpha;
    snake->beta = beta;
    snake->gamma = gamma;

    int n = contourSize(con);
    snake->mat.resize(n * n);
    fillMatrixSnake(*snake);
}

static double getForceOnPointX(
        struct snake& snake, 
        unsigned int x, 
        unsigned int y) {

    return dip::SubpixelLocation(
        snake.exteng.dip_force.TensorRow(0), 
        { x, y }, 
        dip::S::MAXIMUM,
        dip::S::LINEAR).value;
}

static double getForceOnPointY(
        struct snake& snake,
        unsigned int x, 
        unsigned int y) {

    return dip::SubpixelLocation(
        snake.exteng.dip_force.TensorRow(1), 
        { x, y }, 
        dip::S::MAXIMUM,
        dip::S::LINEAR).value;
}

static void updateContour(
        struct snake& snake, 
        const std::vector<double> fex, 
        const std::vector<double> fey) {

    int n = contourSize(&snake.con);
    double new_conx[n];
    double new_cony[n];
    for (int i = 0; i < n; i++) {
        double sumx = 0.0;
        double sumy = 0.0;
        for (int j = 0; j < n; j++) {
            sumx += (
                snake.con.x[j] + snake.gamma * fex[j]
            ) * snake.mat[i * n + j];
            sumy += (
                snake.con.y[j] + snake.gamma * fey[j]
            ) * snake.mat[i * n + j];
        }
        new_conx[i] = sumx;
        new_cony[i] = sumy;
    }
    for (int i = 0; i < n; i++) {
        snake.con.x[i] = new_conx[i];
        snake.con.y[i] = new_cony[i];
    }
}

EXTERNC void snakeExec(struct snake *snake, int niter = 50) {
    int n = contourSize(&snake->con);
    std::vector<double> fex(n);
    std::vector<double> fey(n);
    for (int i = 0; i < niter; i++) {
        for (int j = 0; j < n; j++) {
            unsigned int xcood = round(snake->con.x[j]);
            unsigned int ycood = round(snake->con.y[j]);
            fex[j] = getForceOnPointX(*snake, xcood, ycood);
            fey[j] = getForceOnPointY(*snake, xcood, ycood);
        }
        updateContour(*snake, fex, fex);
    }
}

EXTERNC void snakeFree(struct snake *snake) {
    delete snake;
}

#ifdef SNAKE_STANDALONE
int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "USAGE: " << argv[0] << " [filename]" << std::endl;
        return 1;
    }
    const char *filename = argv[1];

    struct image im;
    struct contour con; 
    struct energy en;
    struct snake snake;

    imageInit(&im);
    imageRead(&im, filename);
    contourInit(&con);
    contourCreate(&con);
    energyInit(&en);
    energyCalculateForce(&en, &im, 30.0);
    snakeInit(&snake, &im, &con, &en, 0.001, 0.4, 100);
    snakeExec(&snake);

    return 0;
}
#endif
