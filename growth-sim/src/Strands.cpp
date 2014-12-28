#include "Strands.h"
#include "Util.h"

using namespace ci;
using namespace std;


StrandBox::StrandBox()
    : mNumSeeds(16),
      mNumStrands(500),
      mStrandLength(60),
      mGrowthProbability(0.019),
      mGrowthDirection(0, 1.25),
      mSpringLength(0.015),
      mSpringIterations(15),
      mSpringK(0.92),
      mStraightenK(0.4),
      mSmoothK(0.02),
      mAlignmentK(0.265),
      mBorderRect(0, 0, 1, 0.5),
      mGridRect(-0.5, -0.5, 1.5, 1.0),
      mGridWidth(160),
      mGridHeight(80)
{
    reset();
}


void StrandBox::reset()
{
    mStrands.clear();
    mStrands.reserve(100);

    for (int i = 0; i < 2; i++) {
        mGrids[i].clear();
        mGrids[i].resize(mGridWidth * mGridHeight);
    }
    mGridCurrent = &mGrids[0];
    mGridNext = &mGrids[1];
    
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

    // Add new seeded strands.
    // We try to keep them ordered from left to right, so we can
    // see patterns on the affinity matrix.

    while (mStrands.size() < mNumStrands) {
    
        unsigned strandsPerSeed = mNumStrands / mNumSeeds;
        unsigned seedIndex = (mStrands.size() / strandsPerSeed) % mNumSeeds;

        float seedY = -0.02;
        float seedRadius = 0.0001;
        Vec2f seedVec(seedIndex / (mNumSeeds - 1.0f), seedY);
        seedVec += mRand.nextVec2f() * seedRadius;

        std::shared_ptr<Strand> newStrand = std::make_shared<Strand>();
        newStrand->seed(seedVec);
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
    vector<GridElement> &grid = *mGridCurrent;

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
            GridElement& element = grid[idxA];

            Vec2f direction = (b - a).safeNormalized();
            float k = 0.1;

            element.flow += direction * k;
        }
    }
}


void StrandBox::smoothGrid()
{
    vector<GridElement> &src = *mGridCurrent;
    vector<GridElement> &dest = *mGridNext;
    float k = mSmoothK / 10.0f;

    for (int y = 0; y < mGridHeight; y++) {
        for (int x = 0; x < mGridWidth; x++) {
            int idx = x + y * mGridWidth;
            Vec2f flow = src[idx].flow;

            if (x > 0) flow += src[idx - 1].flow * k;
            if (y > 0) flow += src[idx - mGridWidth].flow * k;

            if (x + 1 < mGridWidth) flow += src[idx + 1].flow * k;
            if (y + 1 < mGridHeight) flow += src[idx + mGridWidth].flow * k;

            // Limit vector length
            float l = flow.length();
            if (l > 1.0f) {
                flow *= 1.0f / l;
            }

            dest[idx].flow = flow;
        }
    }
    
    swap(mGridCurrent, mGridNext);
}


void StrandBox::alignStrandsWithGrid()
{
    vector<GridElement> &grid = *mGridCurrent;
    float k = mAlignmentK * 1e-3;
    
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];
        vector<Vec2f> &points = strand->getPoints();
        
        for (unsigned i = 1; i < points.size(); i++) {
            Vec2f &a = points[i-1];
            Vec2f &b = points[i];
            
            int idxA = gridIndexFromPoint(a);
            if (idxA < 0) {
                continue;
            }
            GridElement& element = grid[idxA];

            // Apply a balanced force in the same direction as the flow
            Vec2f f = k * element.flow;

            b += f;
            a -= f;
        }
    }
}


int StrandBox::gridIndexFromPoint(ci::Vec2f point)
{
    int xi = (point.x - mGridRect.x1) * mGridWidth  / (mGridRect.x2 - mGridRect.x1);
    int yi = (point.y - mGridRect.y1) * mGridHeight / (mGridRect.y2 - mGridRect.y1);
    if (xi < 0 || yi < 0 || xi >= mGridWidth || yi >= mGridHeight) {
        return -1;
    }
    return mGridWidth * yi + xi;
}


ci::Vec2f StrandBox::pointFromGridIndex(int idx)
{
    int xi = idx % mGridWidth;
    int yi = idx / mGridWidth;
    return Vec2f( mGridRect.x1 + (mGridRect.x2 - mGridRect.x1) * ((xi + 0.5f) / mGridWidth),
                  mGridRect.y1 + (mGridRect.y2 - mGridRect.y1) * ((yi + 0.5f) / mGridHeight) );
}


void StrandBox::draw()
{
    // Background
    gl::color(Color::gray(1.0f));
    gl::drawSolidRect(mBorderRect);

    // Grid squares
    vector<GridElement> &grid = *mGridCurrent;
    for (int i = 0; i < grid.size(); i++) {
        Vec2f center = pointFromGridIndex(i);
        GridElement& element = grid[i];
        
        gl::color(Color(0.9f, 0.7f, 0.7f));
        gl::drawLine(center, center + element.flow * 0.02);
    }
    
    // Strands
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];

        gl::color(Color::gray(0.0f));
        gl::draw(strand->getPolyLine());
    }
}


ci::JsonTree StrandBox::serialize()
{
    JsonTree array = JsonTree::makeArray("strands");
    
    for (int i = 0; i < mStrands.size(); i++) {
        array.addChild(mStrands[i]->serialize());
    }
    
    return array;
}


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


JsonTree Strand::serialize()
{
    JsonTree obj = JsonTree::makeObject();

    obj.addChild(toJson("points", getPolyLine()));
    
    return obj;
}

