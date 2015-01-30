// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/svg/Svg.h"
#include "cinder/Exception.h"
#include "cinder/Rand.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"
#include <string>

class CircleWorld {
public:
    void setup(ci::svg::DocRef doc);
    void initColors(ci::ImageSourceRef colorTable);
    void update();
    
    const ci::svg::Node& findNode(const std::string &name);
    ci::Shape2d findShape(const std::string &name);
    ci::Vec2f findMetric(const std::string &name);

    b2Vec2 vecToBox(ci::Vec2f v);
    ci::Vec2f vecFromBox(b2Vec2 v);
    
    const float kMetersPerPoint         = 10.0;
    const unsigned kStepsPerTableRow    = 100;
    const float kMinTriangleArea        = 0.1;
    static const unsigned kMaxParticles = 32768;
    
    ci::svg::DocRef     mSvg;
    ci::TriMesh2d       mObstacles;
    ci::Surface         mColorTable;
    ci::Rand            mRand;
    float               mTriangulatePrecision;
    unsigned            mNewParticleRate;
    float               mNewParticleLifetime;
    bool                mMoveSpinnersRandomly;
    
    std::vector<ci::Vec2f>      mOriginPoints;
    ci::Rectf                   mOriginBounds;
    std::vector<b2Body*>        mSpinnerBodies;
    std::vector<ci::TriMesh2d>  mSpinnerMeshes;
    std::vector<ci::Vec2f>      mLedPoints;
    
    ci::Rectf               mForceGridExtent;
    float                   mForceGridResolution;
    float                   mForceGridStrength;
    unsigned                mForceGridWidth;
    std::vector<ci::Vec2f>  mForceGrid;
    
    b2World				*mB2World;
    b2ParticleSystem    *mParticleSystem;
    uint64_t            mStepNumber;
    unsigned            mCurrentTableRow;
    float               mSubRow;
    
    b2Vec2              mPositionBuffer[kMaxParticles];
    b2Vec2              mVelocityBuffer[kMaxParticles];
    b2ParticleColor     mColorBuffer[kMaxParticles];
    int32               mExpirationTimeBuffer[kMaxParticles];
    bool                mUpdatedSinceLastDraw;
    
    class ExcNodeNotFound : public cinder::Exception {
    public:
        ExcNodeNotFound(const std::string &name) throw();
        virtual const char* what() const throw() { return mMessage; }
    private:
        char mMessage[2048];
    };

private:
    void setupObstacles(const ci::Shape2d& shape);
    void setupStrands(const ci::Shape2d& shape);
    void setupSpinner(const ci::Shape2d& shape);
    void addFixturesForMesh(b2Body *body, ci::TriMesh2d &mesh, float density = 1.0f);
    void newParticle();
    void updateSpinners();
    void applyGridForces();
};
