#include "Strands.h"
#include "nanoflann.h"

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
    mPointLinks.resize(points.size());
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
      mAlignmentK(0.008)
{
    mStrands.reserve(100);
}


void StrandBox::reset()
{
    mStrands.clear();
    mSimulationStep = 0;
}


void StrandBox::update()
{
    mSimulationStep++;

    adjustStrandCount();
    adjustStrandLength();
    integrateStrandForces();
    clusterStrands();
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
        newStrand->seed(Vec2f( mRand.nextFloat(), 0.0f ));
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


// Index tree for 4D points (x0, y0, dx * dk, dy * dk)
struct StrandBoxIndex4D
{
    StrandBoxIndex4D(StrandBox *sb, float dk = 0.1) :
        mStrandEdges(sb->mStrandLength - 1),
        mNumStrands(sb->mStrands.size()),
        mSB(sb),
        mDK(dk),
        mTree(4, *this)
    {
        mTree.buildIndex();
    }
    
    size_t kdtree_get_point_count() const
    {
        return mStrandEdges * mNumStrands;;
    }
    
    size_t combine_index(unsigned strandId, unsigned edgeId)
    {
        return edgeId + strandId * mStrandEdges;
    }
    
    unsigned strandId(size_t idx) const
    {
        return idx / mStrandEdges;
    }
    
    unsigned edgeId(size_t idx) const
    {
        return idx % mStrandEdges;
    }
        
    float kdtree_get_pt(const size_t idx, int dim) const
    {
        std::shared_ptr<Strand> strand = mSB->mStrands[strandId(idx)];
        std::vector<ci::Vec2f> &points = strand->getPoints();
        unsigned e = edgeId(idx);
        
        if (e + 1 >= points.size()) {
            // Point isn't allocated yet. Return a stand-in location.
            return -10.0f;
        }
        
        switch (dim) {
            default:
            case 0: return points[e].x;
            case 1: return points[e].y;
            case 2: return (points[e+1].x - points[e].x) * mDK;
            case 3: return (points[e+1].y - points[e].y) * mDK;
        }
    }
    
    void get_pt_vector(const size_t idx, float *v) const
    {
        for (unsigned dim = 0; dim < 4; dim++) {
            v[dim] = kdtree_get_pt(idx, dim);
        }
    }

    float kdtree_distance(const float *p1, const size_t idx_p2, size_t size) const
    {
        // Default L2 distance
        float d = 0;
        for (unsigned i = 0; i < 4; i++) {
            float r = kdtree_get_pt(idx_p2, i) - p1[i];
            d += r*r;
        }
        return d;
    }

    template <class BBOX>
    bool kdtree_get_bbox(BBOX &bb) const
    {
        return false;
    }

    typedef nanoflann::KDTreeSingleIndexAdaptor<
        nanoflann::L2_Simple_Adaptor< float, StrandBoxIndex4D >, StrandBoxIndex4D, 4> Tree;

    unsigned mStrandEdges;
    unsigned mNumStrands;
    StrandBox *mSB;
    float mDK;
    Tree mTree;
};


void StrandBox::clusterStrands()
{
    if (mStrands.size() < 1) {
        return;
    }

    StrandBoxIndex4D index(this);
    float rMin = mAlignmentRadiusMin * mAlignmentRadiusMin;
    float rMax = mAlignmentRadiusMax * mAlignmentRadiusMax;
    
    for (int strandId = 0; strandId < mStrands.size(); strandId++) {
        std::shared_ptr<Strand> strand = mStrands[strandId];
        vector<Vec2f> &points = strand->getPoints();

        for (unsigned edgeId = 0; edgeId + 1 < points.size(); edgeId++) {
            size_t combinedIndex = index.combine_index(strandId, edgeId);
            
            nanoflann::SearchParams params;
            params.sorted = false;
            float coord[4];
            index.get_pt_vector(combinedIndex, coord);
            vector<pair<size_t, float> > hits;
            index.mTree.radiusSearch(coord, rMax, hits, params);

            // The index search results in a list of potentially-near hits, with an index and distance
            for (unsigned hit = 0; hit < hits.size(); hit++) {

                unsigned hitIndex = hits[hit].first;
                unsigned hitStrandId = index.strandId(hitIndex);
                if (hitStrandId == strandId) {
                    // Same strand
                    continue;
                }

                float relativeDist = (hits[hit].second - rMin) / (rMax - rMin);
                if (relativeDist < 0.0f || relativeDist > 1.0f) {
                    // Too far away or too close
                    continue;
                }
                
                float kernel = 1.0f - (relativeDist * relativeDist);
                kernel = kernel * kernel * kernel;
                float k = kernel * mAlignmentK * 1e-3;
                
                // Push segment at combinedIndex, a little closer to the segment at hitIndex.
                
                std::shared_ptr<Strand> hitStrand = mStrands[hitStrandId];
                std::vector<ci::Vec2f> &hitPoints = hitStrand->getPoints();
                unsigned hitEdgeId = index.edgeId(hitIndex);
                    
                if (hitEdgeId + 1 >= hitPoints.size()) {
                    // Point is unallocated
                }
                
                // Only move the X axis, to keep from bunching togther vertically
                points[edgeId].x += (hitPoints[hitEdgeId].x - points[edgeId].x) * k;
                points[edgeId+1].x += (hitPoints[hitEdgeId+1].x - points[edgeId+1].x) * k;
            }
        }
    }
}


void StrandBox::draw()
{
    for (int i = 0; i < mStrands.size(); i++) {
        std::shared_ptr<Strand> strand = mStrands[i];

        gl::color(Color::gray(0.0f));
        gl::draw(strand->getPolyLine());
    }
}

