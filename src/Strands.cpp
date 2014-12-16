#include "Strands.h"

using namespace ci;
using namespace std;


void Strand::seed(Vec2f point)
{
    vector<Vec2f> &points = getPoints();

    points.clear();
    points.reserve(100);
    points.push_back(point);
}


void Strand::grow(Rand &rand, Vec2f direction, float distance )
{
    vector<Vec2f> &points = getPoints();

    points.push_back( points.back() + (rand.nextVec2f() + direction) * distance );
}


void Strand::springForce(float k, float restingLength)
{
    // For every pair of adjacent points in a strand, push or pull
    // them until the distance between them equals restingLength.
   
    vector<Vec2f> &points = getPoints();

    for (unsigned i = 1; i < points.size(); i++) {
        Vec2f &a = points[i-1];
        Vec2f &b = points[i];
    
        Vec2f ab = b - a;
        float l = ab.length();
        float f = (k * (l - restingLength)) / l;
        
        a += f * ab;
        b -= f * ab;
    }
}


void Strand::straightenForce(float k)
{
    // For every triad of adjacent points (ABC) in a strand,
    // straighten the angle formed at the center point (B) by
    // pulling the end points (AC) apart from each other.
    // This force is proportional to the area of triangle (ABC).

    vector<Vec2f> &points = getPoints();
    
    for (unsigned i = 2; i < points.size(); i++) {
        Vec2f &a = points[i-2];
        Vec2f &b = points[i-1];
        Vec2f &c = points[i];
        
        Vec2f ab = b - a;
        Vec2f ac = c - a;
        float l = ac.length();
        float f = k / l * fabs(ab.x * ac.y - ab.y * ac.x);

        a -= f * ab;
        c += f * ab;
    }

}


StrandBox::StrandBox()
    : mNumStrands(20),
      mStrandLength(180),
      mGrowthProbability(0.01),
      mGrowthDirection(0, 0.68),
      mSpringLength(0.005),
      mSpringIterations(120),
      mSpringK(0.92),
      mStraightenK(0.66)
{
    mStrands.reserve(100);
}


void StrandBox::reset()
{
    mStrands.clear();
}


void StrandBox::update()
{
    while (mStrands.size() > mNumStrands) {
        // Randomly delete excess strands
        mStrands.erase(mStrands.begin() + mRand.nextInt(mStrands.size()));
    }
    
    // Add new seeded strands
    while (mStrands.size() < mNumStrands) {
        mStrands.push_back(Strand());
        Strand &newStrand = mStrands.back();
        newStrand.seed(Vec2f( mRand.nextFloat(), 0.0f ));
    }
    
    // Grow or truncate strands
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];
        std::vector<ci::Vec2f> &points = strand.getPoints();

        while (points.size() > mStrandLength) {
            points.pop_back();
        }
        
        if (points.size() < mStrandLength
            && mRand.nextFloat() >= mGrowthProbability) {
            strand.grow(mRand, mGrowthDirection);
        }
    }
        
    // Integrate forces within each strand
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];
        for (int step = 0; step < mSpringIterations; step++) {
            strand.springForce(mSpringK, mSpringLength);
            strand.straightenForce(mStraightenK);
        }
    }
}


void StrandBox::draw()
{
    for (int i = 0; i < mStrands.size(); i++) {
        Strand &strand = mStrands[i];

        gl::color(Color::gray(0.0f));
        gl::draw(strand.getPolyLine());
    }
}
