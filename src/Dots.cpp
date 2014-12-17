#include "Dots.h"

using namespace ci;
using namespace std;


Dots::Dots(StrandBox &sb) :
    mSB(sb),
    mNumDots(20),
    mSmallestDotSize(1.0 / 16.0),
    mLargestDotSize(1.5 / 16.0)
{
    reset();
}


float Dots::getDotSize(int idx)
{
    return mSmallestDotSize + (mLargestDotSize - mSmallestDotSize) * idx / (mNumDots - 1);
}


void Dots::reset()
{
    mDotPoints.clear();
}


void Dots::update()
{
    adjustDotCount();
    edgeForces();
    dotSpringForces();
}


void Dots::adjustDotCount()
{
    while (mDotPoints.size() > mNumDots) {
        // Randomly delete excess dots
        mDotPoints.erase(mDotPoints.begin() + mSB.mRand.nextInt(mDotPoints.size()));
    }
    
    // Seed new dots randomly near the center
    while (mDotPoints.size() < mNumDots) {
        
        float seedRadius = 0.0001;
        Vec2f seedVec = mSB.mBorderRect.getCenter();
        seedVec += mSB.mRand.nextVec2f() * seedRadius;

        mDotPoints.push_back(seedVec);
    }
}


void Dots::edgeForces()
{

}


void Dots::dotSpringForces()
{
}


void Dots::draw()
{
    gl::color(0,0,1,0.2);
    gl::enableAlphaBlending();

    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        Vec2f point = mDotPoints[dot];
        float size = getDotSize(dot);
        
        gl::drawSolidCircle(point, size);
    }

    gl::disableAlphaBlending();
}
