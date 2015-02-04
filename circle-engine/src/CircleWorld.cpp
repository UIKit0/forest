// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/Rand.h"
#include "cinder/Perlin.h"
#include "CircleWorld.h"
#include <stdio.h>

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

    b2Vec2 gravity( 0.0f, vecToBox(findMetric("gravity")).y );
    mB2World = new b2World(gravity);

    Vec2f metricParticleRadius = findMetric("particle-radius");
    mTriangulatePrecision = 1.0f / metricParticleRadius.y;
    
    b2ParticleSystemDef particleSystemDef;
    particleSystemDef.colorMixingStrength = 0.01;
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

    mNewParticleRate = 18;
    mNewParticleLifetime = 20.0;
    mMoveSpinnersRandomly = false;
    
    setupObstacles(findShape("obstacles"));
    setupStrands(findShape("strands"));

    for (int index = 0;; index++) {
        char name[64];
        snprintf(name, sizeof name, "spinner-%d", index);
        const svg::Node* node = mSvg->findNode(name);
        if (node) {
            setupSpinner(node->getShape());
        } else {
            break;
        }
    }

    for (int index = 0;; index++) {
        char name[64];
        snprintf(name, sizeof name, "led-%d", index);
        const svg::Node* node = mSvg->findNode(name);
        if (node) {
            mLedPoints.push_back(node->getShape().calcBoundingBox().getCenter());
        } else {
            break;
        }
    }
}

void CircleWorld::initColors(ci::ImageSourceRef colorTable)
{
    mColorTable = colorTable;
    mCurrentTableRow = 0;
    mSubRow = 0;
    mStepNumber = 0;
}

void CircleWorld::setupObstacles(const Shape2d& shape)
{
    // Store a triangulated mesh, for drawing quickly
    Triangulator triangulator(shape, mTriangulatePrecision);
    mObstacles = triangulator.calcMesh();
    
    b2BodyDef groundBodyDef;
    b2Body *ground = mB2World->CreateBody(&groundBodyDef);
    addFixturesForMesh(ground, mObstacles, 0.0f);
}

void CircleWorld::setupStrands(const Shape2d& shape)
{
    mOriginPoints.clear();
    
    mForceGridExtent = shape.calcBoundingBox();
    mForceGridResolution = findMetric("force-grid-resolution").y;
    mForceGridStrength = findMetric("force-grid-strength").y * 1e-3;
    mForceGridWidth = mForceGridExtent.getWidth() / mForceGridResolution;
    mForceGrid.resize(mForceGridWidth * int(mForceGridExtent.getHeight() / mForceGridResolution));
    
    for (unsigned contour = 0; contour < shape.getNumContours(); contour++) {
        const Path2d &path = shape.getContour(contour);
        unsigned steps = path.calcLength() / mForceGridResolution * 4.0;

        mOriginPoints.push_back(path.getPosition(0.0f));

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

    // Bounding rectangle of all origin points
    mOriginBounds = mOriginPoints;
}

void CircleWorld::setupSpinner(const ci::Shape2d& shape)
{
    Vec2f center = shape.calcPreciseBoundingBox().getCenter();

    Triangulator triangulator(shape.transformCopy(MatrixAffine2f::makeTranslate(-center)),
                              mTriangulatePrecision);
    TriMesh2d mesh = triangulator.calcMesh();
    mSpinnerMeshes.push_back(mesh);
    
    b2BodyDef bodyDef;
    bodyDef.position = vecToBox(center);

    b2Body *obj = mB2World->CreateBody(&bodyDef);
    mSpinnerBodies.push_back(obj);
    addFixturesForMesh(obj, mesh, 0.0f);
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

    for (unsigned i = 0; i < mNewParticleRate; i++) {
        newParticle();
    }

    applyGridForces();

    mStepNumber++;
    mCurrentTableRow = (mStepNumber / kStepsPerTableRow) % mColorTable.getHeight();
    mSubRow = (mStepNumber % kStepsPerTableRow) / float(kStepsPerTableRow - 1);

    mB2World->Step( 1 / 60.0f, 1, 1, 2 );
    mUpdatedSinceLastDraw = true;
}

void CircleWorld::updateSpinners(midi::Hub& midi)
{
    if (mMoveSpinnersRandomly) {
        Perlin p(4, 0);
        for (unsigned i = 0; i < mSpinnerBodies.size(); i++) {
            b2Body *spinner = mSpinnerBodies[i];
            float angle = p.fBm(mStepNumber * 0.002, i) * 50.0;
            spinner->SetTransform(spinner->GetPosition(), angle);
        }

    } else {
        ci::midi::Message msg;
        while (midi.getNextMessage(&msg)) {

            if (msg.channel >= 1 && msg.channel <= 8 && msg.byteOne == 0x0a) {
                // One hardcoded spinner input, for debugging color cube
                static int temp[8];
                temp[msg.channel-1] = msg.byteTwo;
                if (msg.channel == 8) {
                    float r = temp[0] | (temp[1] << 7);
                    float g = temp[2] | (temp[3] << 7);
                    float b = temp[4] | (temp[5] << 7);
                    float c = temp[6] | (temp[7] << 7);
                    mSpinnerColorCube.push(r, g, b);
                }

            } else {
                // Other unhandled message
                printf("[midi] port=%d ch=%d type=%02x value=%02x\n",
                       msg.port, msg.channel, msg.byteOne, msg.byteTwo);
            }
        }
    }
}

void CircleWorld::applyGridForces()
{
    b2Vec2* positions = mParticleSystem->GetPositionBuffer();
    b2Vec2* velocities = mParticleSystem->GetVelocityBuffer();
    unsigned numParticles = mParticleSystem->GetParticleCount();
    float scale = 1.0f / (mForceGridResolution * kMetersPerPoint);
    b2Vec2 gridCorner = vecToBox(mForceGridExtent.getUpperLeft());
    
    for (unsigned i = 0; i < numParticles; i++) {
        b2Vec2 pos = (positions[i] - gridCorner) * scale;
        int ix = pos.x;
        unsigned idx = ix + int(pos.y) * mForceGridWidth;
        if (ix < mForceGridWidth && idx < mForceGrid.size()) {
            velocities[i] += vecToBox(mForceGrid[idx]) * mForceGridStrength;
        }
    }
}

void CircleWorld::newParticle()
{
    b2ParticleDef pd;
    Vec2f pos = mOriginPoints[mRand.nextInt(mOriginPoints.size())];

    float x = (pos.x - mOriginBounds.getX1()) / mOriginBounds.getWidth();
    Vec2i loc( x * (mColorTable.getWidth() - 1), mCurrentTableRow );

    auto pix1 = mColorTable.getPixel(loc);
    loc.y = (loc.y + 1) % mColorTable.getHeight();
    auto pix2 = mColorTable.getPixel(loc);
    float a1 = 1.0f - mSubRow;
    float a2 = mSubRow;
    
    pd.position = vecToBox(pos);
    pd.flags = b2_colorMixingParticle | b2_tensileParticle;
    pd.lifetime = mNewParticleLifetime;

    pd.color.Set(pix1.r * a1 + pix2.r * a2,
                 pix1.g * a1 + pix2.g * a2,
                 pix1.b * a1 + pix2.b * a2, 255);

    mParticleSystem->CreateParticle(pd);
}
