// Color cube pointcloud history (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ColorCubePoints.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;

ColorCubePoints::ColorCubePoints(unsigned maxPoints)
    : mMaxPoints(maxPoints),
      mNextPoint(0)
{}

    
void ColorCubePoints::add(Vec3f v)
{
    mNextPoint %= mMaxPoints;
    if (mNextPoint >= mPoints.size()) {
        mPoints.resize(mNextPoint+1);
    }
    mPoints[mNextPoint] = v;
    mNextPoint++;
}

void ColorCubePoints::add(float r, float g, float b)
{
    add(Vec3f(r,g,b));
}
    
void ColorCubePoints::draw()
{
    gl::color(0.0f, 0.0f, 0.0f);
    gl::drawCube(Vec3f(0.5f, 0.5f, 0.5f), Vec3f(1.0f, 1.0f, 1.0f));

    gl::color(0.4f, 0.4f, 0.7f);
    gl::drawStrokedCube(Vec3f(0.5f, 0.5f, 0.5f), Vec3f(1.0f, 1.0f, 1.0f));

    if (!mPoints.size()) {
        return;
    }

    AxisAlignedBox3f range(mPoints[0], mPoints[0]);
    for (unsigned i = 1; i < mPoints.size(); i++) {
        range.include(AxisAlignedBox3f(mPoints[i], mPoints[i]));
    }

    gl::enableAdditiveBlending();
    gl::color(1.0f, 1.0f, 1.0f);
    glPointSize(4.0f);

    glBegin(GL_POINTS);
    for (unsigned i = 0; i < mPoints.size(); i++) {
        Vec3f p = (mPoints[i] - range.getMin()) / range.getSize();
        gl::color(p.x, p.y, p.z);
        gl::vertex(p);
    }
    glEnd();
}
