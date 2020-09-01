#ifndef SNAKE_H
#define SNAKE_H

xt::xtensor<double, 2> create_constmat(double alpha, double beta, unsigned int n);
void calc_gradforce_x();
void calc_gradforce_y();

#endif