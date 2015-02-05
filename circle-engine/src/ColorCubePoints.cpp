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
    mLineSolver.solve(mPoints);
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
    Vec4f local = mLineSolver.worldToLocal * getCurrentPoint();
    return atan2f(local.y, local.x);
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

    gl::enableAdditiveBlending();
    gl::color(1.0f, 1.0f, 1.0f);
    
    AxisAlignedBox3f range = getRange();
    gl::pushModelView();
    gl::scale(Vec3f(1.0f, 1.0f, 1.0f) / range.getSize());
    gl::translate(-range.getMin());

    // Point history
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (unsigned i = 0; i < mPoints.size(); i++) {
        drawColorPoint(range, mPoints[i]);
    }
    glEnd();

    // Local coordinates for latest point
    Vec4f local = mLineSolver.worldToLocal * Vec4f(getCurrentPoint(), 1.0f);

    // Local coordinates for arbitrary endpoints on the Z axis
    Vec4f zMin(0.0f, 0.0f, range.getMin().z, 1.0f);
    Vec4f zMax(0.0f, 0.0f, range.getMax().z, 1.0f);
    
    // Draw a triangle between the Z axis and current point
    gl::color(1.0f, 1.0f, 1.0f);
    gl::drawLine((mLineSolver.localToWorld * zMin).xyz(),
                 (mLineSolver.localToWorld * zMax).xyz());
    gl::color(1.0f, 0.0f, 0.0f);
    gl::drawLine((mLineSolver.localToWorld * zMin).xyz(),
                 (mLineSolver.localToWorld * local).xyz());
    gl::color(0.0f, 1.0f, 0.0f);
    gl::drawLine((mLineSolver.localToWorld * zMax).xyz(),
                 (mLineSolver.localToWorld * local).xyz());

    gl::popModelView();
}

void ColorCubePoints::drawColorPoint(const AxisAlignedBox3f& range, Vec3f p)
{
    Vec3f scaled = (p - range.getMin()) / range.getSize();
    gl::color(scaled.x, scaled.y, scaled.z);
    gl::vertex(p);
}

LineSolver::LineSolver()
{
    // Initial solution
    result.x0 = 0.0f;
    result.y0 = 0.0f;
    result.xz = 1.0f;
    result.yz = 1.0f;
}

void LineSolver::solve(vector<Vec3f> &points)
{
    const int m = points.size();    // Number of functions to minimize
    const int n = 5;                // Number of independent variables

    vector<int> iwa(n);                // Integer work array
    vector<float> wa(m*n + 5*n + m);   // Working array
    vector<float> residuals(m);
    
    // Default tolerance: sqaure root of machine precision
    float tol = sqrt(sdpmpar(1));

    // Minimize the system of equations
    slmdif1(&minFunc, &points[0], m, n,
          &result.array[0], &residuals[0],
          tol, &iwa[0], &wa[0], (int)wa.size());

    // Local coordinate system has a stable XY plane perpendicular to the line, and Z along the line.
    // X and Y axes are defined relative to Z, to be the same length.
    
    Vec3f origin(result.x0, result.y0, 0.0f);
    Vec3f zAxis(result.xz, result.yz, 1.0f);
    float zScale = 1.0f / zAxis.length();

    Vec3f up(0.0f, 1.0f, 0.0f);
    Vec3f xAxis = zAxis.cross(up).normalized() * zScale;
    Vec3f yAxis = xAxis.cross(zAxis).normalized() * zScale;

    localToWorld.setColumn(0, Vec4f(xAxis, origin.x));
    localToWorld.setColumn(1, Vec4f(yAxis, origin.y));
    localToWorld.setColumn(2, Vec4f(zAxis, origin.z));
    localToWorld.setColumn(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));

    worldToLocal = localToWorld.affineInverted();
}

int LineSolver::minFunc(void *p, int m, int n, const float *x, float *fvec, int flag)
{
    Vec3f* points = (Vec3f*) p;
    const Vec& param = *(const Vec*)x;
    Vec3f x1(param.x0, param.y0, 0.0f);
    Vec3f x2 = x1 + Vec3f(param.xz, param.yz, 1.0f);
    
    for (int i = 0; i < m; i++) {
        Vec3f point = points[i];
        fvec[i] = (point - x1).cross(point - x2).lengthSquared() / (x2 - x1).lengthSquared();
    }

    return flag;
}
