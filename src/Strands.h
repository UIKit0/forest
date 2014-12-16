#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "cinder/PolyLine.h"
#include <vector>


class Strand
{
public:
    void seed(ci::Vec2f point);
    void grow(ci::Rand &rand, ci::Vec2f direction, float distance = 0.001);

    void springForce(float k, float restingLength);
    void straightenForce(float k);

    const ci::PolyLine2f&   getPolyLine() const { return mSpine; }
    ci::PolyLine2f&         getPolyLine() { return mSpine; }
    
    const std::vector<ci::Vec2f>&    getPoints() const { return mSpine.getPoints(); }
    std::vector<ci::Vec2f>&          getPoints() { return mSpine.getPoints(); }
    
private:
    ci::PolyLine2f  mSpine;
};


class StrandBox
{
public:
    StrandBox();

    void reset();
    void update();
    void draw();
    
    unsigned                mNumStrands;
    unsigned                mStrandLength;
    float                   mGrowthProbability;
    ci::Vec2f               mGrowthDirection;
    
    float                   mSpringLength;
    unsigned                mSpringIterations;
    float                   mSpringK;
    float                   mStraightenK;
    
    std::vector<Strand>     mStrands;
    ci::Rand                mRand;
};
