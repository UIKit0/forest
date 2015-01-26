#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/svg/Svg.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CircleEngineApp : public AppNative {
public:
    void prepareSettings( Settings *settings );
    void setup();
    void mouseDrag( MouseEvent event );
    void update();
    void draw();

    void drawParticles();
    void drawObstacles();
    
    const float kMetersPerPoint         = 1e3;
    const float kMinTriangleArea        = 0.1;
    const float kParticleRadiusPoints   = 4.0;
    
    svg::DocRef         mSvg;
    TriMesh2d           mObstacles;
    
    b2World				*mWorld;
    b2ParticleSystem    *mParticleSystem;

    gl::GlslProg        mParticleShader;
    GLuint              mParticleShaderPosition;
    GLuint              mParticleShaderColor;
};

void CircleEngineApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 120.0f );
    settings->setWindowSize( 1280, 720 );
}


void CircleEngineApp::setup()
{
    // Everything our engine needs comes in SVG format, editable in Adobe Illustrator
    mSvg = svg::Doc::create(loadAsset("world.svg"));

    b2Vec2 gravity( 0.0f, 0.0f );
    mWorld = new b2World( gravity );

    // Tesselate the "obstacles" layer into a triangular mesh
    auto obstacles = mSvg->findNode("obstacles");
    if (!obstacles) throw std::runtime_error("No 'obstacles' layer in world SVG");
    Triangulator obstacleTri(obstacles->getShape(), 0.25f);
    mObstacles = obstacleTri.calcMesh();

    b2BodyDef groundBodyDef;
    b2Body *ground = mWorld->CreateBody(&groundBodyDef);

    // Import the obstacle triangles into Box2D
    int numFixtures = 0;
    for (size_t tri = 0; tri < mObstacles.getNumTriangles(); tri++) {
        Vec2f triVerts[3];
        mObstacles.getTriangleVertices(tri, &triVerts[0], &triVerts[1], &triVerts[2]);
        float area = 0.5f * (triVerts[2] - triVerts[0]).cross(triVerts[1] - triVerts[0]);
        if (area > kMinTriangleArea) {
            b2Vec2 b2Verts[3];
            for (unsigned v = 0; v < 3; v++) {
                b2Verts[v].Set( kMetersPerPoint * triVerts[v].x, kMetersPerPoint * triVerts[v].y );
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
    mParticleSystem = mWorld->CreateParticleSystem(&particleSystemDef);
    mParticleSystem->SetGravityScale(0.4f);
    mParticleSystem->SetDensity(1.2f);
    mParticleSystem->SetRadius( kMetersPerPoint * kParticleRadiusPoints );

    mParticleShader = gl::GlslProg( loadAsset("particle.glslv"), loadAsset("particle.glslf") );
    mParticleShader.bind();
    mParticleShaderPosition = mParticleShader.getAttribLocation("position");
    mParticleShaderColor = mParticleShader.getAttribLocation("color");
}

void CircleEngineApp::mouseDrag( MouseEvent event )
{
    Vec2f pos = event.getPos();

    b2CircleShape shape;
    shape.m_p.Set( kMetersPerPoint * pos.x, kMetersPerPoint * pos.y );
    shape.m_radius = kMetersPerPoint * 10.0f;
    b2Transform xf;
    xf.SetIdentity();
    
    b2ParticleGroupDef pd;
    pd.shape = &shape;
    pd.flags = b2_colorMixingParticle | b2_tensileParticle;
    pd.color.Set(255, 255, 0, 192);
    mParticleSystem->CreateParticleGroup(pd);
}

void CircleEngineApp::update()
{
    mWorld->Step( 1 / 30.0f, 10, 10 );
}

void CircleEngineApp::draw()
{
    gl::setMatricesWindow( getWindowSize() );
    gl::disable(GL_DEPTH_TEST);
    gl::disable(GL_CULL_FACE);

    gl::clear( Color( 0, 0, 0 ) );
    
    drawParticles();
    drawObstacles();
}

void CircleEngineApp::drawObstacles()
{
    gl::disableAlphaBlending();
    gl::color( Color( 0.33f, 0.33f, 0.33f ) );
    gl::draw(mObstacles);
}

void CircleEngineApp::drawParticles()
{
    int particleCount = mParticleSystem->GetParticleCount();
    const b2Vec2* positionBuffer = mParticleSystem->GetPositionBuffer();
    const b2ParticleColor* colorBuffer = mParticleSystem->GetColorBuffer();

    gl::enableAdditiveBlending();

    mParticleShader.bind();
    glPointSize( mParticleSystem->GetRadius() / kMetersPerPoint );
    glPushMatrix();
    glScalef(1.0f/kMetersPerPoint, 1.0f/kMetersPerPoint, 1.0f/kMetersPerPoint);
    glVertexAttribPointer(mParticleShaderPosition, 2, GL_FLOAT, GL_FALSE, 0, &positionBuffer[0].x);
    glVertexAttribPointer(mParticleShaderColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuffer[0].r);
    glEnableVertexAttribArray(mParticleShaderPosition);
    glEnableVertexAttribArray(mParticleShaderColor);
    glDrawArrays(GL_POINTS, 0, particleCount);
    glDisableVertexAttribArray(mParticleShaderPosition);
    glDisableVertexAttribArray(mParticleShaderColor);
    glPopMatrix();
    mParticleShader.unbind();
}

CINDER_APP_NATIVE( CircleEngineApp, RendererGl )
