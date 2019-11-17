#pragma once

#include <eigen3/Eigen/Dense>
namespace eig = Eigen;

struct Particle
{
    double mass;
    double radius;

    eig::Vector2d position;
    eig::Vector2d velocity;
    eig::Vector2d acceleration = eig::Vector2d::Zero();
};
