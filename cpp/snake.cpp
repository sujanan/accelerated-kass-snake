#include <iostream>
#include "xtensor/xtensor.hpp"
#include "xtensor/xbuilder.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xmanipulation.hpp"

#include "snake.h"

xt::xtensor<double, 2> create_constmat(double alpha, double beta, unsigned int n)
{
    xt::xtensor<double, 1> row = xt::zeros<double>({n});
    row[  0] = -2 * alpha - 6 * beta;
    row[  1] = alpha + 4 * beta;
    row[  2] = -beta;
    row[n-2] = -beta;
    row[n-1] = alpha + 4 * beta;
    xt::xtensor<double, 2> constmat = xt::empty<double>({n, n});
    for (unsigned int i = 0; i < n; i++) {
        xt::row(constmat, i) = xt::roll(row, i);
    }
    return constmat;
}

