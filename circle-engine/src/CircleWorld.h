// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/svg/Svg.h"
#include "cinder/Exception.h"
#include "cinder/Rand.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "cinder/Timer.h"
#include "Box2D/Box2D.h"
#include "MidiHub.h"
#include "ColorCubePoints.h"
#include <string>

class CircleWorld {
public:
    void setup(ci::svg::DocRef doc);
    void initColors(ci::ImageSourceRef colorTable);
    void update(ci::midi::Hub& midi);
    void newParticle();
    void particleBurst();
    void clearColorCubes();
    
    const ci::svg::Node& findNode(const std::string &name);
    ci::Shape2d findShape(const std::string &name);
    ci::Vec2f findMetric(const std::string &name);

    b2Vec2 vecToBox(ci::Vec2f v);
    ci::Vec2f vecFromBox(b2Vec2 v);
    
    const float kMetersPerPoint         = 10.0;
    const float kMinTriangleArea        = 0.1;
    static const unsigned kMaxParticles = 32768;

    struct Spinner {
        b2Body*           mBody;
        b2RevoluteJoint*  mJoint;
        ci::TriMesh2d     mMesh;
        float             mTargetAngle;
        ColorCubePoints   mColorCube;
        uint8_t           mColorPacket[8];

        void handleMidi(const ci::midi::Message& msg);
        void sensorAngle(float angle, float motorPower);
    };
    
    ci::svg::DocRef     mSvg;
    ci::TriMesh2d       mObstacles;
    ci::TriMesh2d       mFrontLayer;
    ci::Surface         mColorTable;
    b2PolygonShape      mVacuumShape;
    ci::Rectf           mFeedbackRect;
    ci::Rand            mRand;
    ci::Timer           mTimer;
    float               mTriangulatePrecision;
    unsigned            mMaxParticleRate;
    float               mMaxParticleLifetime;
    bool                mOneSpinnerControlsAll;
    bool                mMoveSpinnersRandomly;
    float               mSpinnerPower;
    
    std::vector<ci::Vec2f>  mLedPoints;
    std::vector<Spinner>    mSpinners;
    std::vector<ci::Rectf>  mSourceRects;
    
    ci::Rectf               mForceGridExtent;
    float                   mForceGridResolution;
    float                   mForceGridStrength;
    unsigned                mForceGridWidth;
    std::vector<ci::Vec2f>  mForceGrid;
    ci::Surface32f          mForceGridSurface;
    
    b2World				*mB2World;
    b2Body              *mGround;
    b2ParticleSystem    *mParticleSystem;
    uint64_t            mStepNumber;
    unsigned            mCurrentTableRow;
    float               mSubRow;
    unsigned            mStepsPerTableRow;
    
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
    void setupFrontLayer(const ci::Shape2d& shape);
    void setupStrands(const ci::Shape2d& shape);
    void setupSpinner(const ci::Shape2d& shape);
    void setupSource(const ci::Shape2d& shape);
    void setupLed(const ci::Shape2d& shape);
    
    void setupShapeSequence(const char *fmt, std::function<void(const ci::Shape2d &)> handler);
    void setShapeAsConvexHull(b2PolygonShape &poly, const ci::Shape2d& shape);
    void addFixturesForMesh(b2Body *body, ci::TriMesh2d &mesh, float density = 1.0f);
    void updateSpinners(ci::midi::Hub& midi);
    void applyGridForces();
};
