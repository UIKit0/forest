// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "CircleWorld.h"
#include <stdio.h>
#include <functional>
#include <mach/mach.h>
#include <mach/mach_time.h>

using namespace ci;
using namespace std;


CircleWorld::ExcNodeNotFound::ExcNodeNotFound(const string &name) throw()
{
    snprintf(mMessage, sizeof mMessage, "Could not locate required SVG node: %s", name.c_str());
}

const svg::Node& CircleWorld::findNode(const string &name)
{
    const svg::Node *node = mSvg->findNode(name);
    if (!node)
        throw ExcNodeNotFound(name);
    return *node;
}

Shape2d CircleWorld::findShape(const string &name)
{
    return findNode(name).getShapeAbsolute();
}

Vec2f CircleWorld::findMetric(const string &name)
{
    Vec2f v = findNode(name).getBoundingBoxAbsolute().getSize();
    printf("[metric] %s = (%f, %f)\n", name.c_str(), v.x, v.y);
    return v;
}

b2Vec2 CircleWorld::vecToBox(Vec2f v)
{
    return b2Vec2( v.x * kMetersPerPoint, v.y * kMetersPerPoint );
}

Vec2f CircleWorld::vecFromBox(b2Vec2 v)
{
    return Vec2f( v.x / kMetersPerPoint, v.y / kMetersPerPoint );
}

void CircleWorld::setup(svg::DocRef doc)
{
    mSvg = doc;

    mTimer.start();

    b2Vec2 gravity( 0.0f, vecToBox(findMetric("gravity")).y );
    mB2World = new b2World(gravity);

    Vec2f metricParticleRadius = findMetric("particle-radius");
    mTriangulatePrecision = 1.0f / metricParticleRadius.y;
    
    b2ParticleSystemDef particleSystemDef;
    particleSystemDef.colorMixingStrength = 0.05f;
    mParticleSystem = mB2World->CreateParticleSystem(&particleSystemDef);
    mParticleSystem->SetGravityScale(0.4f);
    mParticleSystem->SetDensity(1.2f);
    mParticleSystem->SetRadius(vecToBox(metricParticleRadius).y / 2);
    mParticleSystem->SetDestructionByAge(true);

    // Statically allocated buffers, so we can safely share them across threads
    mParticleSystem->SetPositionBuffer(mPositionBuffer, kMaxParticles);
    mParticleSystem->SetVelocityBuffer(mVelocityBuffer, kMaxParticles);
    mParticleSystem->SetColorBuffer(mColorBuffer, kMaxParticles);
    mParticleSystem->SetExpirationTimeBuffer(mExpirationTimeBuffer, kMaxParticles);

    mMaxParticleRate = 5;
    mMaxParticleLifetime = 300.0;
    mMoveSpinnersRandomly = false;
    mOneSpinnerControlsAll = false;
    mSpinnerPower = 10.0f;
    mForceGridStrength = 2.0f;
    
    setupObstacles(findShape("obstacles"));
    setupFrontLayer(findShape("front-layer"));
    setupStrands(findShape("strands"));
    setShapeAsConvexHull(mVacuumShape, findShape("vacuum"));
    setupShapeSequence("spinner-%d", bind( &CircleWorld::setupSpinner, this, _1 ));
    setupShapeSequence("led-%d", bind( &CircleWorld::setupLed, this, _1 ));
    setupShapeSequence("source-%d", bind( &CircleWorld::setupSource, this, _1 ));
}

void CircleWorld::setupShapeSequence(const char *fmt, std::function<void(const ci::Shape2d &)> handler)
{
    for (int index = 0;; index++) {
        char name[64];
        snprintf(name, sizeof name, fmt, index);
        const svg::Node* node = mSvg->findNode(name);
        if (node) {
            handler(node->getShape());
        } else {
            break;
        }
    }
}

void CircleWorld::initColors(ci::ImageSourceRef colorTable)
{
    mRand.seed(mach_absolute_time());
    mPerlin = Perlin(4, mRand.nextInt());

    mColorChooser.setup(colorTable, &mPerlin, 16);
}

void CircleWorld::setupObstacles(const Shape2d& shape)
{
    // Store a triangulated mesh, for drawing quickly
    Triangulator triangulator(shape, mTriangulatePrecision);
    mObstacles = triangulator.calcMesh();
    
    b2BodyDef groundBodyDef;
    groundBodyDef.type = b2_staticBody;
    mGround = mB2World->CreateBody(&groundBodyDef);
    addFixturesForMesh(mGround, mObstacles, 0.0f);
}

void CircleWorld::setupFrontLayer(const Shape2d& shape)
{
    // High-res triangulated mesh in a VBO
    Triangulator triangulator(shape);
    mFrontLayer = triangulator.calcMesh();
}

void CircleWorld::setupStrands(const Shape2d& shape)
{
    mForceGridExtent = shape.calcBoundingBox();
    mForceGridResolution = findMetric("force-grid-resolution").y;
    mForceGridWidth = mForceGridExtent.getWidth() / mForceGridResolution;
    int forceGridHeight = mForceGridExtent.getHeight() / mForceGridResolution;
    mForceGrid.resize(mForceGridWidth * forceGridHeight);
    
    for (unsigned contour = 0; contour < shape.getNumContours(); contour++) {
        const Path2d &path = shape.getContour(contour);
        unsigned steps = path.calcLength() / mForceGridResolution * 4.0;

        for (unsigned step = 0; step < steps; step++) {
            float t = step / float(steps - 1);
            Vec2f position = path.getPosition(t);
            Vec2f tangent = path.getTangent(t);
            unsigned x = (position.x - mForceGridExtent.getX1()) / mForceGridResolution;
            unsigned y = (position.y - mForceGridExtent.getY1()) / mForceGridResolution;
            unsigned idx = x + y * mForceGridWidth;
            if (x < mForceGridWidth && idx < mForceGrid.size()) {
                mForceGrid[idx] += tangent;
            }
        }
    }

    mForceGridSurface = ci::Surface32f(mForceGridWidth, forceGridHeight, false, SurfaceChannelOrder::RGB);

    for (unsigned y = 0; y < forceGridHeight; y++) {
        for (unsigned x = 0; x < mForceGridWidth; x++) {
            Vec2f force = mForceGrid[x + y * mForceGridWidth];
            mForceGridSurface.setPixel(Vec2i(x,y), Color(force.x, force.y, 0.0f));
        }
    }
}

void CircleWorld::setupSpinner(const ci::Shape2d& shape)
{
    mSpinners.emplace_back();
    Spinner &newSpinner = mSpinners.back();

    Vec2f center = shape.calcPreciseBoundingBox().getCenter();
    Triangulator triangulator(shape.transformCopy(MatrixAffine2f::makeTranslate(-center)),
                              mTriangulatePrecision);
    newSpinner.mMesh = triangulator.calcMesh();
    
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = vecToBox(center);

    newSpinner.mBody = mB2World->CreateBody(&bodyDef);
    addFixturesForMesh(newSpinner.mBody, newSpinner.mMesh);

    b2RevoluteJointDef jointDef;
    jointDef.Initialize(newSpinner.mBody, mGround, bodyDef.position);
    
    newSpinner.mTargetAngle = 0.0f;
    newSpinner.mJoint = dynamic_cast<b2RevoluteJoint*>(mB2World->CreateJoint(&jointDef));
}

void CircleWorld::setupSource(const ci::Shape2d& shape)
{
    mSourceRects.push_back(shape.calcPreciseBoundingBox());
}

void CircleWorld::setupLed(const ci::Shape2d& shape)
{
    mLedPoints.push_back(shape.calcBoundingBox().getCenter());
}

void CircleWorld::setShapeAsConvexHull(b2PolygonShape &poly, const Shape2d& shape)
{
    vector<b2Vec2> boxPoints;
    const vector<Path2d>& contours = shape.getContours();

    for (unsigned i = 0; i < contours.size(); i++) {
        const vector<Vec2f>& points = contours[i].getPoints();
        for (unsigned j = 0; j < points.size(); j++) {
            boxPoints.push_back(vecToBox(points[j]));
        }
    }
    
    poly.Set(&boxPoints[0], boxPoints.size());
}

void CircleWorld::addFixturesForMesh(b2Body *body, ci::TriMesh2d &mesh, float density)
{
    b2PolygonShape polygon;
    for (size_t tri = 0; tri < mesh.getNumTriangles(); tri++) {
        Vec2f v[3];
        mesh.getTriangleVertices(tri, &v[0], &v[1], &v[2]);
        float area = 0.5f * (v[2] - v[0]).cross(v[1] - v[0]);
        if (area > kMinTriangleArea) {
            b2Vec2 bv[3];
            for (unsigned i = 0; i < 3; i++) {
                bv[i] = vecToBox(v[i]);
            }
            polygon.Set(&bv[0], 3);
            body->CreateFixture(&polygon, density);
        }
    }
}

void CircleWorld::update(ci::midi::Hub& midi)
{
    updateSpinners(midi);
    applyGridForces();
    mColorChooser.update();
    
    mB2World->Step( 1 / 60.0f, 1, 1, 2 );
    mUpdatedSinceLastDraw = true;

    b2Transform xf;
    xf.SetIdentity();
    mParticleSystem->DestroyParticlesInShape(mVacuumShape, xf);
}

void CircleWorld::updateSpinners(midi::Hub& midi)
{
    // Track raw colors and build a model that lets us convert to angle
    ci::midi::Message msg;
    while (midi.getNextMessage(&msg)) {

        if (mOneSpinnerControlsAll) {
            // MIDI messages go to all spinners, for debug / simulation
            for (unsigned controller = 0; controller < mSpinners.size(); controller++) {
                mSpinners[controller].handleMidi(msg);
            }

        } else {
            // Dispatch to one spinner according to the message's "controller" byte
            unsigned controller = msg.byteOne;
            if (controller < mSpinners.size()) {
                mSpinners[controller].handleMidi(msg);
            }
        }
    }

    // Update spinner velocities each physics step
    for (unsigned i = 0; i < mSpinners.size(); i++) {
        Spinner& spinner = mSpinners[i];

        if (mMoveSpinnersRandomly) {
            // Random sensor data
            spinner.sensorAngle(mPerlin.fBm(mTimer.getSeconds() * 0.1, i) * 50.0, mSpinnerPower);
        
        } else if (spinner.mColorCube.isAngleReliable()) {
            // Real sensor data
            spinner.sensorAngle(spinner.mColorCube.getCurrentAngle(), mSpinnerPower);

        } else {
            // Disable the motor, just spin freely
            spinner.mJoint->EnableMotor(false);
        }
    }
}

void CircleWorld::Spinner::handleMidi(const midi::Message &msg)
{
    /*
     * Colors come in as 'control change' messages where the 'channel'
     * cycles from 1 to 8 to deliver an 8-byte packet.
     */
    
    if ((msg.status & 0xF0) == 0xB0) {
        // Control change

        if (msg.channel >= 1 && msg.channel <= 8) {
            mColorPacket[msg.channel-1] = msg.byteTwo;
            if (msg.channel == 8) {
                const float s = 1e3;
                float r = mColorPacket[0] | (mColorPacket[1] << 7);
                float g = mColorPacket[2] | (mColorPacket[3] << 7);
                float b = mColorPacket[4] | (mColorPacket[5] << 7);
                float c = mColorPacket[6] | (mColorPacket[7] << 7);
                mColorCube.push(r*s/c, g*s/c, b*s/c);
            }
        }
    }
}

void CircleWorld::Spinner::sensorAngle(float angle, float motorPower)
{
    // The sensor's full scale is one half revolution.
    // Convert that to a continuous angle with no discontinuities.

    angle = M_PI/2 + angle/2;

    float diff = fmod(angle - mTargetAngle, M_PI);
    if (diff >=  M_PI/2) diff -= M_PI;
    if (diff <= -M_PI/2) diff += M_PI;
    mTargetAngle += diff;
    
    float servo = mBody->GetAngle() - mTargetAngle;
    
    if (fabs(servo) < M_PI*4) {
        // Close enough to the target (two rotations) we'll act like a servo motor
    
        // The joint acts like a spring connected between the virtual spinner and the physical one
        mJoint->EnableMotor(true);
        mJoint->SetMotorSpeed(servo * motorPower);

        // These motors have a very high "mass" due to being attached to the obstacle body
        mJoint->SetMaxMotorTorque(1e20);

    } else {
        // Lots of rotation to catch up, warp there instantly
        mJoint->EnableMotor(false);
        mBody->SetTransform(mBody->GetPosition(), mTargetAngle);
    }
}

void CircleWorld::clearColorCubes()
{
    for (unsigned i = 0; i < mSpinners.size(); i++) {
        mSpinners[i].mColorCube.clear();
    }
}

void CircleWorld::applyGridForces()
{
    b2Vec2* positions = mParticleSystem->GetPositionBuffer();
    b2Vec2* velocities = mParticleSystem->GetVelocityBuffer();
    unsigned numParticles = mParticleSystem->GetParticleCount();
    float scale = 1.0f / (mForceGridResolution * kMetersPerPoint);
    b2Vec2 gridCorner = vecToBox(mForceGridExtent.getUpperLeft());
    float gain = mForceGridStrength * 1e-3;
    
    for (unsigned i = 0; i < numParticles; i++) {
        b2Vec2 pos = (positions[i] - gridCorner) * scale;
        int ix = pos.x;
        unsigned idx = ix + int(pos.y) * mForceGridWidth;
        if (ix < mForceGridWidth && idx < mForceGrid.size()) {
            velocities[i] += vecToBox(mForceGrid[idx]) * gain;
        }
    }
}

void CircleWorld::particleBurst()
{
    for (unsigned i = 0; i < mMaxParticleRate; i++) {
        newParticle();
    }
}

void CircleWorld::newParticle()
{
    b2ParticleDef pd;

    // Choose the particle emitter that is closest to fewest existing particles

    vector<unsigned> closestCount(mSourceRects.size());
    unsigned particleCount = mParticleSystem->GetParticleCount();

    fill(closestCount.begin(), closestCount.end(), 0);
    
    for (unsigned i = 0; i < particleCount; i++) {
        Vec2f position = vecFromBox(mPositionBuffer[i]);
        float closestDist2 = INFINITY;
        int closestIndex = -1;
        
        // Measure the X axis only
        for (unsigned j = 0; j < mSourceRects.size(); j++) {
            float d = mSourceRects[j].getCenter().x - position.x;
            float dist2 = d * d;
            if (dist2 < closestDist2) {
                closestDist2 = dist2;
                closestIndex = j;
            }
        }
    
        assert(closestIndex >= 0);
        closestCount[closestIndex]++;
    }
    
    int x = -1;
    unsigned smallestCount = (unsigned)-1;
    
    for (unsigned i = 0; i < mSourceRects.size(); i++) {
        if (closestCount[i] <= smallestCount) {
            smallestCount = closestCount[i];
            x = i;
        }
    }
    
    assert(x >= 0 && x < mSourceRects.size());
    Color pix = mColorChooser.sample(x);
    Rectf rect = mSourceRects[x];
    
    // Random position within the source box
    Vec2f pos(mRand.nextFloat(rect.getX1(), rect.getX2()),
              mRand.nextFloat(rect.getY1(), rect.getY2()));

    pd.position = vecToBox(pos);
    pd.flags = b2_colorMixingParticle | b2_tensileParticle;
    pd.lifetime = mMaxParticleLifetime;

    pd.color.Set(pix.r * 255.0f, pix.g * 255.0f, pix.b * 255.0f, 255);

    mParticleSystem->CreateParticle(pd);
}
