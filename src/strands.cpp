#include "strands.h"

using namespace ci;


void Strand::seed(Vec2d point)
{
    points.clear();
    points.push_back(point);
}


void Strand::grow(Rand &rand, Vec2d direction, double distance )
{
    points.push_back( points.back() + (rand.nextVec2f() + direction) * distance );
}


void Strand::springForce(double k, double restingLength)
{
    // For every pair of adjacent points in a strand, push or pull
    // them until the distance between them equals restingLength.
   
    for (unsigned i = 1; i < points.size(); i++) {
        Vec2d &a = points[i-1];
        Vec2d &b = points[i];
    
        Vec2d ab = b - a;
        double l = ab.length();
        double f = (k * (l - restingLength)) / l;
        
        a += f * ab;
        b -= f * ab;
    }
}


void Strand::straightenForce(double k)
{
    // For every triad of adjacent points (ABC) in a strand,
    // straighten the angle formed at the center point (B) by
    // pulling the end points (AC) apart from each other.
    // This force is proportional to the area of triangle (ABC).

    for (unsigned i = 2; i < points.size(); i++) {
        Vec2d &a = points[i-2];
        Vec2d &b = points[i-1];
        Vec2d &c = points[i];
        
        Vec2d ab = b - a;
        Vec2d ac = c - a;
        double l = ac.length();
        double f = k / l * fabs(ab.x * ac.y - ab.y * ac.x);

        a -= f * ab;
        c += f * ab;
    }
}
