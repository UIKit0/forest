#include "Strands.h"

using namespace ci;
using namespace std;


Strand::Strand()
: mCurrent(&mBuffers[0]),
  mNext(&mBuffers[1])
{}


void Strand::seed(Vec2f point)
{
    vector<Vec2f> &points = getPoints();

    points.clear();
    points.reserve(100);
    points.push_back(point);

    mNext = mCurrent;
}


void Strand::grow(Rand &rand, Vec2f direction, float distance )
{
    vector<Vec2f> &points = getPoints();
    points.push_back( points.back() + (rand.nextVec2f() + direction) * distance );
    mNext = mCurrent;
}


void Strand::springForce(float k, float restingLength)
{
    // For every pair of adjacent points in a strand, push or pull
    // them until the distance between them equals restingLength.
    
    vector<Vec2f> &points = getPoints();
    vector<Vec2f> &nextPoints = mNext->getPoints();
    
    for (unsigned i = 1; i < points.size(); i++) {
        Vec2f a = points[i-1];
        Vec2f b = points[i];
    
        Vec2f ab = b - a;
        float l = ab.length();
        float f = (k * (l - restingLength)) / l;
        
        Vec2f fab = f * ab;
        
        nextPoints[i-1] = a + fab;
        nextPoints[i] = b - fab;
    }

    swap(mCurrent, mNext);
}


void Strand::straightenForce(float k)
{
    // For every triad of adjacent points (ABC) in a strand,
    // straighten the angle formed at the center point (B) by
    // pulling the end points (AC) apart from each other.
    // This force is proportional to the area of triangle (ABC).

    vector<Vec2f> &points = getPoints();
    vector<Vec2f> &nextPoints = mNext->getPoints();
    
    for (unsigned i = 2; i < points.size(); i++) {
        Vec2f a = points[i-2];
        Vec2f b = points[i-1];
        Vec2f c = points[i];
        
        Vec2f ab = b - a;
        Vec2f ac = c - a;
        float l = ac.length();
        float f = k / l * fabs(ab.x * ac.y - ab.y * ac.x);
        Vec2f fab = f * ab;
        
        nextPoints[i-2] = a - fab;
        nextPoints[i-1] = b;
        nextPoints[i] = c + fab;
    }

    swap(mCurrent, mNext);
}


StrandBox::StrandBox()
    : mNumStrands(40),
      mStrandLength(180),
      mGrowthProbability(0.1),
      mGrowthDirection(0, 0.68),
      mSpringLength(0.005),
      mSpringIterations(120),
      mSpringK(0.92),
      mStraightenK(0.66),
      mAlignmentRadiusMin(0.01),
      mAlignmentRadiusMax(0.05),
      mAlignmentK(0.008),
      mRect(0, 0, 1, 0.5),
      mGridWidth(100),
      mGridHeight(100)
{
    reset();
}


void StrandBox::reset()
{
    mStrands.clear();
    mStrands.reserve(100);

    mGrid.clear();
    mGrid.resize(mGridWidth * mGridHeight);
    
    mSimulationStep = 0;
}


void StrandBox::update()
{
    mSimulationStep++;

    adjustStrandCount();
    adjustStrandLength();
    integrateStrandForces();
    updateFlowGrid();
    smoothGrid();
    alignStrandsWithGrid();
}


void StrandBox::adjustStrandCount()
{
    while (mStrands.size() > mNumStrands) {
        // Randomly delete excess strands
        mStrands.erase(mStrands.begin() + mRand.nextInt(mStrands.size()));
    }

    // Add new seeded strands
    while (mStrands.size() < mNumStrands) {
        std::shared_ptr<Strand> newStrand = std::make_shared<Strand>();
        newStrand->seed(Vec2f( mRand.nextFloat(), -0.01f ));
        mStrands.push_back(newStrand);
    }
}


void StrandBox::adjustStrandLength()
{
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];
        std::vector<ci::Vec2f> &points = strand->getPoints();

        while (points.size() > mStrandLength) {
            points.pop_back();
        }
        
        if (points.size() < mStrandLength
            && mRand.nextFloat() <= mGrowthProbability) {
            strand->grow(mRand, mGrowthDirection);
        }
    }
}


void StrandBox::integrateStrandForces()
{
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];

        for (int step = 0; step < mSpringIterations; step++) {
            strand->springForce(mSpringK, mSpringLength);
            strand->straightenForce(mStraightenK);
        }
    }
}


void StrandBox::updateFlowGrid()
{
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];
        vector<Vec2f> &points = strand->getPoints();
        
        for (unsigned i = 1; i < points.size(); i++) {
            Vec2f a = points[i-1];
            Vec2f b = points[i];
            
            int idxA = gridIndexFromPoint(a);
            if (idxA < 0) {
                continue;
            }
            GridElement& element = mGrid[idxA];

            Vec2f flow = (b - a).safeNormalized();
            float k = 0.1;

            // Flow is normalized on update, but not on smoothing
            element.flow += (flow - element.flow.safeNormalized()) * k;
        }
    }
}


void StrandBox::smoothGrid()
{
    float k = 0.001;

    for (int y = 1; y + 1 < mGridHeight; y++) {
        for (int x = 1; x + 1 < mGridWidth; x++) {
            int idx = x + y * mGridWidth;
            Vec2f scaledFlow = mGrid[idx].flow * k;

            mGrid[idx - 1].flow += scaledFlow;
            mGrid[idx + 1].flow += scaledFlow;
            mGrid[idx - mGridWidth].flow += scaledFlow;
            mGrid[idx + mGridHeight].flow += scaledFlow;
        }
    }
}

void StrandBox::alignStrandsWithGrid()
{
}


int StrandBox::gridIndexFromPoint(ci::Vec2f point)
{
    int xi = (point.x - mRect.x1) * mGridWidth / (mRect.x2 - mRect.x1);
    int yi = (point.y - mRect.y1) * mGridWidth / (mRect.y2 - mRect.y1);
    if (xi < 0 || yi < 0 || xi >= mGridWidth || yi >= mGridHeight) {
        return -1;
    }
    return mGridWidth * yi + xi;
}


ci::Vec2f StrandBox::pointFromGridIndex(int idx)
{
    int xi = idx % mGridWidth;
    int yi = idx / mGridWidth;
    return Vec2f( mRect.x1 + (mRect.x2 - mRect.x1) * ((xi + 0.5f) / mGridWidth),
                  mRect.y1 + (mRect.y2 - mRect.y1) * ((yi + 0.5f) / mGridHeight) );
}


void StrandBox::draw()
{
    // Background
    gl::color(Color::gray(1.0f));
    gl::drawSolidRect(mRect);

    // Grid squares
    for (int i = 0; i < mGrid.size(); i++) {
        Vec2f center = pointFromGridIndex(i);
        GridElement& element = mGrid[i];
        
        gl::color(Color(0.9f, 0.7f, 0.7f));
        gl::drawLine(center, center + element.flow.safeNormalized() * 0.01);
    }
    
    // Strands
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];

        gl::color(Color::gray(0.0f));
        gl::draw(strand->getPolyLine());
    }
}

