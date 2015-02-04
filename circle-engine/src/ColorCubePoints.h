// Color cube pointcloud history, for color-to-angle conversion
// (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/Vector.h"
#include <vector>


class ColorCubePoints {
public:
    ColorCubePoints(unsigned maxPoints = 64*1024);
    
    void add(ci::Vec3f v);
    void add(int r, int g, int b);

    //! Render a debug view of the cloud's state, in a normalized [0,1] cube
    void draw();

private:
    std::vector<ci::Vec3f> mPoints;
    unsigned mMaxPoints;
    unsigned mNextPoint;    // Cyclic
};
