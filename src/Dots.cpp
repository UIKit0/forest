#include "Dots.h"

using namespace ci;
using namespace std;


Dots::Dots(StrandBox &sb) :
    mSB(sb),
    mNumDots(30),
    mSmallestDotSize(1.25 / 32.0),
    mLargestDotSize(1.5 / 32.0),
    mDotGravity(1.0),
    mDotSpacing(8.0),
    mDotMaxStrands(5),
    mRepelK(0.04),
    mRetainK(0.36),
    mDecayK(0.08),
    mAttractK(0.3)
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
    mDotStrandAffinity.clear();
}


void Dots::update()
{
    adjustDotCount();
    edgeForces();
    dotSpringForces();
    strandForces();
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
    
    // Make sure the dot/strand affinity array is the right size, randomly init if
    // the dimensions changed.
    
    unsigned dotStrandAffinitySize = mDotPoints.size() * mSB.mStrands.size();
    if (mDotStrandAffinity.size() != dotStrandAffinitySize) {
        mDotStrandAffinity.resize(dotStrandAffinitySize);
        for (unsigned i = 0; i < dotStrandAffinitySize; i++) {
            mDotStrandAffinity[i] = mSB.mRand.nextFloat();
        }
    }
}


void Dots::edgeForces()
{
    float k = 0.9;
    float margin = 0.01;
    Rectf edges = mSB.mBorderRect.inflated(Vec2f(-margin, -margin));

    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        Vec2f &point = mDotPoints[dot];
        float size = getDotSize(dot);
        float t;

        // Left
        t = (edges.x1 - (point.x - size));
        if (t > 0) {
            point.x += t * k;
        }
        
        // Right
        t = (point.x + size) - edges.x2;
        if (t > 0) {
            point.x -= t * k;
        }

        // Top
        t = (edges.y1 - (point.y - size));
        if (t > 0) {
            point.y += t * k;
        }

        // Bottom
        t = (point.y + size) - edges.y2;
        if (t > 0) {
            point.y -= t * k;
        }
        
        // Gravity force
        point.y -= mDotGravity * 1e-3;
    }
}


void Dots::dotSpringForces()
{
    float k = 0.1;

    for (unsigned dot1 = 0; dot1 < mDotPoints.size(); dot1++) {
        Vec2f &point1 = mDotPoints[dot1];
        float size1 = getDotSize(dot1);

        for (unsigned dot2 = dot1+1; dot2 < mDotPoints.size(); dot2++) {
            Vec2f &point2 = mDotPoints[dot2];
            float size2 = getDotSize(dot2);
            
            float centerDistance = (point2 - point1).length();
            float edgeDistance = centerDistance - size1 - size2 - mDotSpacing * 1e-2;

            if (edgeDistance < 0) {
                Vec2f dir = (point2 - point1).normalized();
                Vec2f force = dir * edgeDistance * k;
                point1 += force;
                point2 -= force;
            }
        }
    }
}


void Dots::strandForces()
{
    // Each dot is greedy for control.
    // How many strands intersect this dot?
    // Too many are a burden, randomly push them away.
    // Too few and we should look for friends nearby.
    // Any friends we keep, draw us toward each other.
    
    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        Vec2f &dotPoint = mDotPoints[dot];
        float dotSize = getDotSize(dot);
        float dotSizeSquared = dotSize * dotSize;
        
        // Remember which strands pass through this dot

        vector<unsigned> hits;

        for (unsigned strandId = 0; strandId < mSB.mStrands.size(); strandId++) {
            shared_ptr<Strand> strand = mSB.mStrands[strandId];
            vector<Vec2f> &strandPoints = strand->getPoints();
            for (unsigned i = 0; i < strandPoints.size(); i++) {
                Vec2f d = strandPoints[i] - dotPoint;
                if (d.lengthSquared() < dotSizeSquared) {
                    hits.push_back(strandId);
                    break;
                }
            }
        }
        
        // Is this dot overwhelmed? If we have more associations than we can handle,
        // sort by descending affinity and diminish all below the top N.

        if (hits.size() > mDotMaxStrands) {
            vector<pair<float, unsigned> > sortBuffer;
            float k = mRepelK * 1e-3;
            
            for (unsigned strandId = 0; strandId < mSB.mStrands.size(); strandId++) {
                float &affinity = mDotStrandAffinity[dot * mSB.mStrands.size() + strandId];
                sortBuffer.push_back(make_pair(-affinity, strandId));
            }
            sort(sortBuffer.begin(), sortBuffer.end());

            for (unsigned i = mDotMaxStrands; i < sortBuffer.size(); i++) {
                unsigned strandId = sortBuffer[i].second;
                float &affinity = mDotStrandAffinity[dot * mSB.mStrands.size() + strandId];
                affinity += (-1.0f - affinity) * k;
            }
        }
        
        // Any friendly strands that are hitting this dot will have their affinities nonlinearly boosted
        
        for (unsigned hitId = 0; hitId < hits.size(); hitId++) {
            unsigned strandId = hits[hitId];
            float &affinity = mDotStrandAffinity[dot * mSB.mStrands.size() + strandId];
            if (affinity > 0.0f) {
                float k = affinity * mRetainK * 1e-2;
                affinity += (1.0f - affinity) * k;
            }
        }

        // Apply force based on affinities and distance

        for (unsigned strandId = 0; strandId < mSB.mStrands.size(); strandId++) {
            float &affinity = mDotStrandAffinity[dot * mSB.mStrands.size() + strandId];
            shared_ptr<Strand> strand = mSB.mStrands[strandId];
            vector<Vec2f> &strandPoints = strand->getPoints();

            affinity -= affinity * mDecayK * 1e-2;
            float k = affinity * mAttractK * 1e-5;

            for (unsigned i = 1; i < strandPoints.size(); i++) {
                Vec2f d = strandPoints[i] - dotPoint;
                d.normalize();

                strandPoints[i] -= d * k;
                dotPoint += d * k;
            }
        }
    }
}


void Dots::draw()
{
    gl::enableAlphaBlending();
    
    // All dots
    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        Vec2f &dotPoint = mDotPoints[dot];
        float dotSize = getDotSize(dot);
        
        gl::color(0.7, 0.7, 1.0, 0.8);
        gl::drawSolidCircle(dotPoint, dotSize, 64);
    }
    
    gl::disableAlphaBlending();
}


void Dots::drawAffinityMatrix()
{
    gl::enableAlphaBlending();

    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        for (unsigned strandId = 0; strandId < mSB.mStrands.size(); strandId++) {
            float affinity = mDotStrandAffinity[dot * mSB.mStrands.size() + strandId];
            
            if (affinity > 0) {
                gl::color(0.0, 0.4, 0.0, 0.33);
            } else {
                gl::color(0.4, 0.0, 0.0, 0.33);
            }

            gl::drawSolidCircle( Vec2f(5, 5)
                                 + dot * Vec2f(0, 5)
                                 + strandId * Vec2f(5, 0),
                                 fabsf(affinity) * 6,
                                 16 );
        }
    }

    gl::disableAlphaBlending();
}
