#pragma once

#include "cinder/Vector.h"
#include "cinder/Rand.h"
#include "Strands.h"
#include <vector>


// Simulation for many dots that interact with each other in a StrandBox
class Dots
{
public:
    Dots(StrandBox &sb);
    
    void reset();
    void update();
    void draw();
    
    float getDotSize(int idx);

    StrandBox& mSB;
    unsigned   mNumDots;
    float      mSmallestDotSize;
    float      mLargestDotSize;
    float      mDotGravity;
    float      mDotSpacing;
    unsigned   mDotMinStrands;
    unsigned   mDotMaxStrands;
    float      mRepelSpacing;
    float      mRepelK;
    float      mAttractK;
    float      mRetainK;

    std::vector<ci::Vec2f>  mDotPoints;

private:
    void adjustDotCount();
    void edgeForces();
    void dotSpringForces();
    void strandForces();
};
