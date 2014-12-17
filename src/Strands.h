#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/PolyLine.h"
#include <vector>


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
    
private:
    ci::PolyLine2f   mBuffers[2];
    ci::PolyLine2f*  mCurrent;
    ci::PolyLine2f*  mNext;
};


typedef std::vector<std::shared_ptr<Strand> > StrandVector;


// Simulation for many strands that interact with each other
class StrandBox
{
public:
    StrandBox();
    
    void reset();
    void update();
    void draw();
    
    unsigned        mNumSeeds;
    unsigned        mNumStrands;
    unsigned        mStrandLength;
    float           mGrowthProbability;
    ci::Vec2f       mGrowthDirection;
    
    float           mSpringLength;
    unsigned        mSpringIterations;
    float           mSpringK;
    float           mStraightenK;
    float           mSmoothK;
    float           mAlignmentK;

    unsigned        mSimulationStep;
    StrandVector    mStrands;
    ci::Rand        mRand;
    ci::Rectf       mBorderRect;
    ci::Rectf       mGridRect;
    
    struct GridElement {
        ci::Vec2f flow;     // Magnitude <= 1.0f
    };

    unsigned mGridWidth, mGridHeight;
    std::vector<GridElement> mGrids[2];
    std::vector<GridElement> *mGridCurrent;
    std::vector<GridElement> *mGridNext;

private:
    void adjustStrandCount();
    void adjustStrandLength();
    void integrateStrandForces();
    void updateFlowGrid();
    void smoothGrid();
    void alignStrandsWithGrid();

    int gridIndexFromPoint(ci::Vec2f point);
    ci::Vec2f pointFromGridIndex(int idx);
};
