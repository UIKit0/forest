// Color cube pointcloud history, for color-to-angle conversion
// (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/Cinder.h"
#include "cinder/CinderMath.h"
#include "cinder/Vector.h"
#include "cinder/Matrix44.h"
#include <vector>


class ColorCubePoints {
public:
    ColorCubePoints(unsigned maxPoints = 32 * 16);

    void clear();

    void push(ci::Vec3f v);
    void push(float r, float g, float b);
    
    //! Render a debug view of the cloud's state, in a normalized [0,1] cube
    void draw();
    
    std::vector<ci::Vec3f>& getPoints();
    const ci::AxisAlignedBox3f& getRangeRGB() const;
    const ci::AxisAlignedBox3f& getRangeXYZ() const;
    
    ci::Vec3f getCurrentPoint() const;
    float getCurrentAngle() const;
    bool isAngleReliable() const;

    float getAngleForPoint(ci::Vec3f point) const;
    
private:
    // Solver for locating a 3D line that passes through the rotational axis in RGB space
    class LineSolver {
    public:
        LineSolver();

        void solve(std::vector<ci::Vec3f> &points);

        struct Vec {
            void setDefault();
            union {
                float array[4];
                struct {
                    float x0, y0, xz, yz;
                };
            };
        };
        
        Vec             result;
        ci::Matrix44f   localToWorld;
        ci::Matrix44f   worldToLocal;
        
    private:
        static int minFunc(void *p, int m, int n, const float *x, float *fvec, int flag);
    };

    float mXYThreshold;
    float mZLimit;
    unsigned mMaxPoints;
    
    LineSolver mLineSolver;

    ci::Vec3f mCurrentPoint;
    std::vector<ci::Vec3f> mPoints;
    ci::AxisAlignedBox3f mRangeRGB;
    ci::AxisAlignedBox3f mRangeXYZ;

    void drawColorPoint(const ci::AxisAlignedBox3f& range, ci::Vec3f p);
    void balance(int numParts = 16);
    void calcRange();
};

