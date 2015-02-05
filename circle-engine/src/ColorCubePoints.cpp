// Color cube pointcloud history (c) 2015 Micah Elizabeth Scott
// MIT license

#include "ColorCubePoints.h"
#include "cinder/AxisAlignedBox.h"
#include "cinder/gl/gl.h"
#include "cminpack.h"

using namespace ci;
using namespace std;

ColorCubePoints::ColorCubePoints(unsigned maxPoints)
    : mMaxPoints(maxPoints)
{}

void ColorCubePoints::push(Vec3f v)
{
    mCurrentPoint = v;
    mPoints.push_back(v);
    balance();
    mLineSolver.solve(mPoints);
}

void ColorCubePoints::balance(int numParts)
{
    // Divide the angle circle into numParts components,
    // and remove the oldest points from any component
    // that has more than its fair share.

    // Count up how many points are in each part

    const int maxPerPart = mMaxPoints / numParts;
    float angleStep = 2*M_PI / numParts;
    vector<int> counts(numParts);
    vector<short> partId(mPoints.size());
    fill(counts.begin(), counts.end(), 0);

    for (unsigned i = 0; i < mPoints.size(); i++) {
        // Calculate the angle only once per balance()
        int id = getAngleForPoint(mPoints[i]) / angleStep;
        if (id < 0) id += numParts;
        assert(id >= 0 && id < numParts);
        partId[i] = id;
        counts[id]++;
    }

    // Second pass, clean up excess points

    unsigned dst = 0;
    for (unsigned src = 0; src < mPoints.size(); src++) {
        int id = partId[src];
        if (counts[id]-- <= maxPerPart) {
            mPoints[dst++] = mPoints[src];
        }
    }
    mPoints.resize(dst);
}

void ColorCubePoints::push(float r, float g, float b)
{
    push(Vec3f(r,g,b));
}

void ColorCubePoints::clear()
{
    mLineSolver = LineSolver();
    mPoints.clear();
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
    return getAngleForPoint(getCurrentPoint());
}

float ColorCubePoints::getAngleForPoint(Vec3f point)
{
    Vec4f local = mLineSolver.worldToLocal * point;
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
    gl::enableAlphaBlending();
    gl::color(0.0f, 0.0f, 0.0f, 0.5f);
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

    // Convert to world coords and clamp to the AABB to keep the chart tidy
    Ray ray((mLineSolver.localToWorld * zMin).xyz(),
            (mLineSolver.localToWorld * (zMax - zMin)).xyz());
    float intersections[2];
    range.intersect(ray, intersections);

    // Draw a triangle between the Z axis and current point
    gl::color(1.0f, 1.0f, 1.0f);
    gl::drawLine(ray.calcPosition(intersections[0]),
                 ray.calcPosition(intersections[1]));
    gl::color(1.0f, 0.0f, 0.0f);
    gl::drawLine(ray.calcPosition(intersections[0]),
                 (mLineSolver.localToWorld * local).xyz());
    gl::color(0.0f, 1.0f, 0.0f);
    gl::drawLine(ray.calcPosition(intersections[1]),
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
    result.setDefault();
    localToWorld.setToIdentity();
    worldToLocal.setToIdentity();
}

void LineSolver::Vec::setDefault()
{
    x0 = 0.0f;
    y0 = 0.0f;
    xz = 1.0f;
    yz = 1.0f;
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

    // If solution is out of range, start over
    float limit0 = 4.0f;
    if (result.x0 < -limit0 || result.x0 > limit0 ||
        result.y0 < -limit0 || result.y0 > limit0) {
        result.setDefault();
    }
    
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
