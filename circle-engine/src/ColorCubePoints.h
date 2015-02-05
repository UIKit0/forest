// Color cube pointcloud history, for color-to-angle conversion
// (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/Vector.h"
#include "cinder/Matrix44.h"
#include <vector>


class LineSolver {
public:
    LineSolver();

    void solve(std::vector<ci::Vec3f> &points);

    struct Vec {
        void setDefault();
        union {
            float array[4];
            struct {
                float x0;
                float y0;
                float xz;
                float yz;
            };
        };
    };
    
    Vec             result;
    ci::Matrix44f   localToWorld;
    ci::Matrix44f   worldToLocal;
    
private:
    static int minFunc(void *p, int m, int n, const float *x, float *fvec, int flag);
};


class ColorCubePoints {
public:
    ColorCubePoints(unsigned maxPoints = 64 * 128);

    void clear();

    void push(ci::Vec3f v);
    void push(float r, float g, float b);
    
    //! Render a debug view of the cloud's state, in a normalized [0,1] cube
    void draw();
    
    std::vector<ci::Vec3f>& getPoints();
    ci::AxisAlignedBox3f getRange();
    ci::Vec3f getCurrentPoint();
    float getCurrentAngle();
    float getAngleForPoint(ci::Vec3f point);
    
private:
    std::vector<ci::Vec3f> mPoints;
    ci::Vec3f mCurrentPoint;
    unsigned mMaxPoints;
    LineSolver mLineSolver;
    
    void drawColorPoint(const ci::AxisAlignedBox3f& range, ci::Vec3f p);
    void balance(int numParts = 64);
};

