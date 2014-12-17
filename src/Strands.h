#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/PolyLine.h"
#include <vector>


// Packed representation of one point on one strand
struct StrandPoint
{
    uint32_t mPacked;
    
    StrandPoint(uint32_t packed = 0)
        : mPacked(packed) {}
   
    StrandPoint(unsigned strandId, unsigned pointId)
        : mPacked(((strandId+1) << 16) | (pointId + 1)) {}
    
    void setNull() {
        mPacked = 0;
    }
    
    bool isNull() const {
        return mPacked == 0;
    }

    unsigned getStrandId() const {
        return (mPacked >> 16) - 1;
    }

    unsigned getPointId() const {
        return (mPacked & 0xffff) - 1;
    }
};


// Simulation object for one strand
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

    StrandPoint& nextPoint(unsigned pointId) { return mPointLinks[pointId]; }
    
private:
    ci::PolyLine2f   mBuffers[2];
    ci::PolyLine2f*  mCurrent;
    ci::PolyLine2f*  mNext;

    std::vector<StrandPoint> mPointLinks;
};


// Simulation for many strands that interact with each other
class StrandBox
{
public:
    StrandBox();
    
    void reset();
    void update();
    void draw();
    
    unsigned        mNumStrands;
    unsigned        mStrandLength;
    float           mGrowthProbability;
    ci::Vec2f       mGrowthDirection;
    
    float           mSpringLength;
    unsigned        mSpringIterations;
    float           mSpringK;
    float           mStraightenK;
    
    float           mAlignmentRadiusMin;
    float           mAlignmentRadiusMax;
    float           mAlignmentK;
    
    typedef std::vector<std::shared_ptr<Strand> > StrandVector;

    uint64_t        mSimulationStep;
    StrandVector    mStrands;
    ci::Rand        mRand;

private:
    void adjustStrandCount();
    void adjustStrandLength();
    void integrateStrandForces();
    void clusterStrands();
};
