#include <istream>
#include <fstream>
#include <iostream>
#include "xtensor/xio.hpp"
#include "xtensor/xnpy.hpp"
#include "xtensor/xtensor.hpp"
#include "opencv2/opencv.hpp"
#include <raylib.h>

#define NITERS 50

#define PROGRAM_NAME "acseq"

cv::Mat xtensor_to_mat(xt::xtensor<float, 2> tensor)
{
    return cv::Mat(
        2,
        std::vector<int>(tensor.shape().begin(), tensor.shape().end()).data(),
        CV_32FC1,
        tensor.data());
}

xt::xtensor<float, 2> mat_to_xtensor(cv::Mat mat)
{
    return xt::adapt(
        (float *)mat.data,
        mat.cols * mat.rows,
        xt::no_ownership(),
        std::vector<int> {mat.rows, mat.cols});
}

xt::xtensor<float, 2> add_gaussian_blur(xt::xtensor<float, 2> img)
{
    cv::Mat src = xtensor_to_mat(img);
    cv::Mat dest = src.clone();
    cv::GaussianBlur(src, dest, {0, 0}, 30);
    return mat_to_xtensor(dest);
}

int main(int argc, char **argv)
{
    SetConfigFlags(FLAG_VSYNC_HINT);

    if (argc != 2) {
        std::cout << "usage: " PROGRAM_NAME " [image].npy" << std::endl;
        return 1;
    }
    std::string img_filename(argv[1]);
    std::string tex_filename(img_filename.substr(0, img_filename.find(".")) + ".png");

    xt::xtensor<float, 2> img = xt::load_npy<float>(img_filename);

    float alpha = 0.001;
    float beta = 0.4;
    float gamma = 100;

    xt::xtensor<float, 2> conx = xt::empty<float>({NITERS, 63});
    xt::xtensor<float, 2> cony = xt::empty<float>({NITERS, 63});
    unsigned int i;
    float ang;
    i = 0;
    ang = 0.0;
    for (; i < conx.size(); i++) {
        conx(0, i) = 120 + 50 * std::cos(ang);
        ang += 0.1;
    }
    i = 0;
    ang = 0.0;
    for (; i < cony.size(); i++) {
        cony(0, i) = 140 + 60 * std::sin(ang);
        ang += 0.1;
    }
    img = add_gaussian_blur(img);
    std::cout << img(0, 0) << std::endl;


    return 0;
}