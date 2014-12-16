#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include <vector>


class Strand
{
public:
    std::vector<ci::Vec2d> points;

    void seed(ci::Vec2d point);
    void grow(ci::Rand &rand, ci::Vec2d direction, double distance = 0.001);

    void springForce(double k, double restingLength);
    void straightenForce(double k);
};
