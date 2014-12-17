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
  
    void drawAffinityMatrix();
    
    float getDotSize(int idx);

    StrandBox& mSB;
    unsigned   mNumDots;
    float      mSmallestDotSize;
    float      mLargestDotSize;
    float      mDotGravity;
    float      mDotSpacing;
    unsigned   mDotMaxStrands;
    float      mRepelK;
    float      mRetainK;
    float      mDecayK;
    float      mAttractK;

    std::vector<ci::Vec2f>      mDotPoints;
    std::vector<float>          mDotStrandAffinity;
    
private:
    void adjustDotCount();
    void edgeForces();
    void dotSpringForces();
    void strandForces();
};
