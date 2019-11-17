#pragma once

#include "Common.hh"

struct Particle
{
    double mass;
    double radius;

    eig::Vector2d position;
    eig::Vector2d velocity;
    eig::Vector2d acceleration = eig::Vector2d::Zero();
};
