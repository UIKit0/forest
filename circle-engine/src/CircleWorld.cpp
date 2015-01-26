// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

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

void CircleWorld::setup(ci::svg::DocRef doc)
{
    mSvg = doc;
    
    b2Vec2 gravity( 0.0f, vecToBox(findMetric("gravity")).y );
    mB2World = new b2World(gravity);
    
    Triangulator obstacleTri(findShape("obstacles"), 0.25f);
    mObstacles = obstacleTri.calcMesh();
    
    b2BodyDef groundBodyDef;
    b2Body *ground = mB2World->CreateBody(&groundBodyDef);
    
    // Import the obstacle triangles into Box2D
    int numFixtures = 0;
    for (size_t tri = 0; tri < mObstacles.getNumTriangles(); tri++) {
        Vec2f triVerts[3];
        mObstacles.getTriangleVertices(tri, &triVerts[0], &triVerts[1], &triVerts[2]);
        float area = 0.5f * (triVerts[2] - triVerts[0]).cross(triVerts[1] - triVerts[0]);
        if (area > kMinTriangleArea) {
            b2Vec2 b2Verts[3];
            for (unsigned v = 0; v < 3; v++) {
                b2Verts[v] = vecToBox(triVerts[v]);
            }
            b2PolygonShape triDef;
            triDef.Set(b2Verts, 3);
            ground->CreateFixture(&triDef, 0.0f);
            numFixtures++;
        }
    }
    printf("[obstacles] %d total triangles, %d in fixture\n",
           (int)mObstacles.getNumTriangles(), numFixtures );

    const b2ParticleSystemDef particleSystemDef;
    mParticleSystem = mB2World->CreateParticleSystem(&particleSystemDef);
    mParticleSystem->SetGravityScale(0.4f);
    mParticleSystem->SetDensity(1.2f);
    mParticleSystem->SetRadius(vecToBox(findMetric("particle-radius")).y);
    mParticleSystem->SetDestructionByAge(true);
}
