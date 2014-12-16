#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/PolyLine.h"
#include <vector>


class Strand
{
public:
    Strand();

    void seed(ci::Vec2f point);
    void grow(ci::Rand &rand, ci::Vec2f direction, float distance = 0.001);

    void springForce(float k, float restingLength);
    void straightenForce(float k);

    const ci::PolyLine2f&   getPolyLine() const { return *mCurrent; }
    ci::PolyLine2f&         getPolyLine() { return *mCurrent; }
    
    const std::vector<ci::Vec2f>&    getPoints() const { return mCurrent->getPoints(); }
    std::vector<ci::Vec2f>&          getPoints() { return mCurrent->getPoints(); }
    
private:
    ci::PolyLine2f   mBuffers[2];
    ci::PolyLine2f*  mCurrent;
    ci::PolyLine2f*  mNext;
};


class StrandBox
{
public:
    StrandBox();
    
    void reset();
    void update();
    void draw();
    
    typedef std::vector<std::shared_ptr<Strand> > StrandVector;
    
    unsigned        mNumStrands;
    unsigned        mStrandLength;
    float           mGrowthProbability;
    ci::Vec2f       mGrowthDirection;
    
    float           mSpringLength;
    unsigned        mSpringIterations;
    float           mSpringK;
    float           mStraightenK;
    
    uint64_t        mSimulationStep;
    StrandVector    mStrands;
    ci::Rand        mRand;
};
