#include <istream>
#include <fstream>
#include <iostream>
#include "xtensor/xio.hpp"
#include "xtensor/xnpy.hpp"
#include "xtensor/xtensor.hpp"
#include <raylib.h>

#include "snake.h"

#define PROGRAM_NAME "acpar"

void pick_init_points(unsigned int win_height,
                      unsigned int win_width,
                      std::string tex_filename,
                      std::vector<float>& xpoints,
                      std::vector<float>& ypoints)
{
    InitWindow(win_height, win_width, PROGRAM_NAME);
    Texture2D tex = LoadTextureFromImage(LoadImage(tex_filename.c_str()));

    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawTexture(tex, 0, 0, WHITE);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 pos = GetMousePosition();
            xpoints.push_back(pos.x);
            ypoints.push_back(pos.y);
        } else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
            break;
        }
        if (xpoints.size() > 0) {
            for (unsigned int i = 0; i < xpoints.size(); i++) {
                DrawCircle(xpoints[i], ypoints[i], 2.5, GREEN);
            }
        }
        EndDrawing();
    }
}

xt::xtensor<double, 1> create_circle_conx(float x1, float y1, float x2, float y2)
{
    float x0 = (x1+x2)/2;
    float y0 = (y1+y2)/2;
    float r = std::sqrt(std::pow(x1-x0, 2) + std::pow(y1-y0, 2));

    xt::xtensor<float, 1> xcon = xt::empty<float>({63});
    float ang = 0.0;
    unsigned int i = 0;
    for (; i < xcon.size(); i++, ang += 0.1) {
        xcon[i] = x0 + r * std::cos(ang);
    }
    return xcon;
}

xt::xtensor<double, 1> create_circle_cony(float x1, float y1, float x2, float y2)
{
    float x0 = (x1+x2)/2;
    float y0 = (y1+y2)/2;
    float r = std::sqrt(std::pow(x1-x0, 2) + std::pow(y1-y0, 2));

    xt::xtensor<float, 1> ycon = xt::empty<float>({63});
    float ang = 0.0;
    unsigned int i = 0;
    for (; i < ycon.size(); i++, ang += 0.1) {
        ycon[i] = y0 + r * std::sin(ang);
    }
    return ycon;
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

    std::vector<float> xpoints;
    std::vector<float> ypoints;

    pick_init_points(img.shape()[0], img.shape()[1], tex_filename, xpoints, ypoints);

    std::vector<xt::xtensor<double, 1>> xcons;
    std::vector<xt::xtensor<double, 1>> ycons;
    unsigned int i;
    unsigned int j;
    for (i = 0; i < xpoints.size(); i++) {
        j = (i+1) % xpoints.size();
        xcons.push_back(create_circle_conx(xpoints[i], ypoints[i], xpoints[j], ypoints[j]));
        ycons.push_back(create_circle_cony(xpoints[i], ypoints[i], xpoints[j], ypoints[j]));
    }

    return 0;
}