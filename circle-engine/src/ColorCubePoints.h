// Color cube pointcloud history, for color-to-angle conversion
// (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/Vector.h"
#include "cinder/Ray.h"
#include <vector>


class CylinderSolver {
public:
    CylinderSolver();

    void solve(std::vector<ci::Vec3f> &points);

    struct Vec {
        union {
            float array[5];
            struct {
                float radius;
                float x0;
                float y0;
                float xz;
                float yz;
            };
        };
        
        ci::Ray getRay() const;
    };
    
    Vec result;

private:
    static int minFunc(void *p, int m, int n, const float *x, float *fvec, int flag);
};


class ColorCubePoints {
public:
    ColorCubePoints(unsigned maxPoints = 64*1024);
    
    void push(ci::Vec3f v);
    void push(float r, float g, float b);
    
    //! Render a debug view of the cloud's state, in a normalized [0,1] cube
    void draw();
    
    std::vector<ci::Vec3f>& getPoints();
    ci::AxisAlignedBox3f getRange();
    ci::Vec3f getCurrentPoint();
    float getCurrentAngle();
    
private:
    std::vector<ci::Vec3f> mPoints;
    ci::Vec3f mCurrentPoint;
    unsigned mMaxPoints;
    unsigned mNextPoint;    // Cyclic
    
    CylinderSolver mCylinder;
};

