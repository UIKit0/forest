// Color cube pointcloud history (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ColorCubePoints.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/gl/gl.h"
#include "cminpack.h"

using namespace ci;
using namespace std;

ColorCubePoints::ColorCubePoints(unsigned maxPoints)
    : mMaxPoints(maxPoints),
      mNextPoint(0)
{}

void ColorCubePoints::push(Vec3f v)
{
    mNextPoint %= mMaxPoints;
    if (mNextPoint >= mPoints.size()) {
        mPoints.resize(mNextPoint+1);
    }
    mPoints[mNextPoint] = v;
    mNextPoint++;
    
    mCurrentPoint = v;
    mCylinder.solve(mPoints);
}

void ColorCubePoints::push(float r, float g, float b)
{
    push(Vec3f(r,g,b));
}

vector<Vec3f>& ColorCubePoints::getPoints()
{
    return mPoints;
}

ci::Vec3f ColorCubePoints::getCurrentPoint()
{
    return mCurrentPoint;
}

float ColorCubePoints::getCurrentAngle()
{
    return 0;
}

AxisAlignedBox3f ColorCubePoints::getRange()
{
    if (mPoints.size()) {
        AxisAlignedBox3f range(mPoints[0], mPoints[0]);
        for (unsigned i = 1; i < mPoints.size(); i++) {
            range.include(AxisAlignedBox3f(mPoints[i], mPoints[i]));
        }
        return range;
    }
    return AxisAlignedBox3f();
}

void ColorCubePoints::draw()
{
    gl::color(0.0f, 0.0f, 0.0f);
    gl::drawCube(Vec3f(0.5f, 0.5f, 0.5f), Vec3f(1.0f, 1.0f, 1.0f));

    gl::color(0.4f, 0.4f, 0.7f);
    gl::drawStrokedCube(Vec3f(0.5f, 0.5f, 0.5f), Vec3f(1.0f, 1.0f, 1.0f));

    AxisAlignedBox3f range = getRange();
    
    // Point history
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

    // Latest point
    gl::enableAdditiveBlending();
    gl::color(1.0f, 1.0f, 1.0f);
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    {
        Vec3f p = (getCurrentPoint() - range.getMin()) / range.getSize();
        gl::color(p.x, p.y, p.z);
        gl::vertex(p);
    }
    glEnd();

    // Center ray of solved cylinder
    Ray cylRay = mCylinder.result.getRay();
    gl::color(1.0f, 1.0f, 1.0f);
    {
        float f1, f2;
        cylRay.calcPlaneIntersection(range.getMin(), Vec3f(1,0,0), &f1);
        cylRay.calcPlaneIntersection(range.getMax(), Vec3f(1,0,0), &f2);
        Vec3f p1 = (cylRay.calcPosition(f1) - range.getMin()) / range.getSize();
        Vec3f p2 = (cylRay.calcPosition(f2) - range.getMin()) / range.getSize();
        printf("%f,%f,%f   %f,%f,%f\n", p1.x,p1.y,p1.z,p2.x,p2.y,p2.z);
        gl::drawLine(p1, p2);
    }
}


CylinderSolver::CylinderSolver()
{
    // Initial solution
    result.x0 = 0.0f;
    result.y0 = 0.0f;
    result.xz = 1.0f;
    result.yz = 1.0f;
    result.radius = 100.0f;
}

void CylinderSolver::solve(vector<Vec3f> &points)
{
    const int m = points.size();    // Number of functions to minimize
    const int n = 5;                // Number of independent variables

    vector<int> iwa(n);                // Integer work array
    vector<float> wa(m*n + 5*n + m);  // Working array
    vector<float> residuals(m);
    
    // Default tolerance: sqaure root of machine precision
    float tol = sqrt(sdpmpar(1));

    // Minimize the system of equations
    slmdif1(&minFunc, &points[0], m, n,
          &result.array[0], &residuals[0],
          tol, &iwa[0], &wa[0], (int)wa.size());
}

int CylinderSolver::minFunc(void *p, int m, int n, const float *x, float *fvec, int flag)
{
    Vec3f* points = (Vec3f*) p;
    const Vec& param = *(const Vec*)x;
    Ray ray = param.getRay();
    Vec3f x1 = ray.getOrigin();
    Vec3f x2 = ray.calcPosition(1.0f);
    
    for (int i = 0; i < m; i++) {
        Vec3f point = points[i];

        // Distance to ray
        float dist = (point - x1).cross(point - x2).length() / (x2 - x1).length();

        // Squared distance to cylinder surface
        float dr = dist;// - param.radius;
        fvec[i] = dr * dr;
    }

    return flag;
}

Ray CylinderSolver::Vec::getRay() const
{
    return Ray( Vec3f(x0, y0, 0.0f), Vec3f(xz, yz, 1.0f) );
}
