#include "Dots.h"

using namespace ci;
using namespace std;


Dots::Dots(StrandBox &sb) :
    mSB(sb),
    mNumDots(20),
    mSmallestDotSize(1.0 / 32.0),
    mLargestDotSize(1.5 / 32.0),
    mDotGravity(0.1),
    mDotSpacing(6.4),
    mDotMinStrands(9),
    mDotMaxStrands(20),
    mRepelSpacing(1.4),
    mRepelK(0.1),
    mAttractK(0.1),
    mRetainK(0.5)
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
        
        // Remember which strands pass through this dot.

        StrandVector hits;

        for (unsigned strandId = 0; strandId < mSB.mStrands.size(); strandId++) {
            shared_ptr<Strand> strand = mSB.mStrands[strandId];
            vector<Vec2f> &strandPoints = strand->getPoints();
            for (unsigned i = 0; i < strandPoints.size(); i++) {
                Vec2f d = strandPoints[i] - dotPoint;
                if (d.lengthSquared() < dotSizeSquared) {
                    hits.push_back(strand);
                    break;
                }
            }
        }
        
        // If we have too many hits, choose strands at random to push away
        
        while (hits.size() > mDotMaxStrands) {
            unsigned hitId = mSB.mRand.nextInt(hits.size());
            shared_ptr<Strand> strand = hits[hitId];
            hits.erase(hits.begin() + hitId);

            vector<Vec2f> &strandPoints = strand->getPoints();
            float k = mRepelK * 1e-3;

            for (unsigned i = 0; i < strandPoints.size(); i++) {
                Vec2f d = strandPoints[i] - dotPoint;
                float l2 = d.lengthSquared();
                float t = l2 / (mRepelSpacing * dotSizeSquared);
                
                if (t < 1.0f && l2 > 0.0f) {
                    // This point is closeby, push it away
                    Vec2f f = (d / sqrtf(l2)) * (k * (1.0 - t));
                    strandPoints[i] += f;
                    dotPoint -= f;
                }
            }
        }
        
        // The remaining ones are pulled closer toward the center with a constant force

        for (unsigned hitId = 0; hitId < hits.size(); hitId++) {
            shared_ptr<Strand> strand = hits[hitId];
            vector<Vec2f> &strandPoints = strand->getPoints();
            float k = mRetainK * 1e-4;
            
            for (unsigned i = 0; i < strandPoints.size(); i++) {
                Vec2f d = strandPoints[i] - dotPoint;
                d.normalize();

                strandPoints[i] -= d * k;
                dotPoint += d * k;
            }
        }

        // If we need more strands, randomly search

        if (hits.size() < mDotMinStrands) {
            float k = mAttractK * 1e-2;
            dotPoint += k * mSB.mRand.nextVec2f();
        }
    }
}


void Dots::draw()
{
    gl::enableAlphaBlending();
    
    for (unsigned dot = 0; dot < mDotPoints.size(); dot++) {
        Vec2f &dotPoint = mDotPoints[dot];
        float dotSize = getDotSize(dot);
        
        gl::color(0,0,1,0.2);
        gl::drawSolidCircle(dotPoint, dotSize, 64);
    }

    gl::disableAlphaBlending();
}
