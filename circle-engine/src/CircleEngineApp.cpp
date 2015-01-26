// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/svg/Svg.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"
#include "CircleWorld.h"

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
    
    CircleWorld         mWorld;

    gl::GlslProg        mParticleShader;
    GLuint              mParticleShaderPosition;
    GLuint              mParticleShaderColor;
};

void CircleEngineApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 60.0f );
    settings->setWindowSize( 1280, 720 );
}

void CircleEngineApp::setup()
{
    mWorld.setup(svg::Doc::create(loadAsset("world.svg")));

    mParticleShader = gl::GlslProg( loadAsset("particle.glslv"), loadAsset("particle.glslf") );
    mParticleShader.bind();
    mParticleShaderPosition = mParticleShader.getAttribLocation("position");
    mParticleShaderColor = mParticleShader.getAttribLocation("color");
}

void CircleEngineApp::mouseDrag( MouseEvent event )
{
    b2CircleShape shape;
    shape.m_p = mWorld.vecToBox(event.getPos());
    shape.m_radius = mWorld.kMetersPerPoint * 10.0f;
    b2Transform xf;
    xf.SetIdentity();
    
    b2ParticleGroupDef pd;
    pd.shape = &shape;
    pd.flags = b2_colorMixingParticle | b2_tensileParticle;
    pd.color.Set(255, 255, 0, 20);
    mWorld.mParticleSystem->CreateParticleGroup(pd);
}

void CircleEngineApp::update()
{
    mWorld.mB2World->Step( 1 / 60.0f, 1, 1, 1 );
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
    gl::draw(mWorld.mObstacles);
}

void CircleEngineApp::drawParticles()
{
    auto& particles = *mWorld.mParticleSystem;
    int particleCount = particles.GetParticleCount();
    const b2Vec2* positionBuffer = particles.GetPositionBuffer();
    const b2ParticleColor* colorBuffer = particles.GetColorBuffer();

    gl::enableAdditiveBlending();

    mParticleShader.bind();
    glPointSize( 4.0f * particles.GetRadius() / mWorld.kMetersPerPoint );
    glPushMatrix();
    glScalef(1.0f/mWorld.kMetersPerPoint, 1.0f/mWorld.kMetersPerPoint, 1.0f/mWorld.kMetersPerPoint);
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
