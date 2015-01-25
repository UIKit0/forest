#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"

#include <Box2D/Box2D.h>

using namespace ci;
using namespace ci::app;
using namespace std;

class CircleEngineApp : public AppNative {
public:
    void setup();
    void mouseDrag( MouseEvent event );
    void update();
    void draw();

    void drawParticles();
    
    const float kPhysicsScale = 1.0 / 2.0f;

    b2World				*mWorld;
    b2ParticleSystem    *mParticleSystem;

    gl::GlslProg        mParticleShader;
    GLuint              mParticleShaderPosition;
    GLuint              mParticleShaderColor;
};

void CircleEngineApp::setup()
{
    b2Vec2 gravity( 0.0f, 0.0f );
    mWorld = new b2World( gravity );
    
    const b2ParticleSystemDef particleSystemDef;
    mParticleSystem = mWorld->CreateParticleSystem(&particleSystemDef);
    mParticleSystem->SetGravityScale(0.4f);
    mParticleSystem->SetDensity(1.2f);
    
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set( 0.0f, kPhysicsScale * getWindowHeight() );
    b2Body* groundBody = mWorld->CreateBody(&groundBodyDef);
    
    // Define the ground box shape.
    b2PolygonShape groundBox;
    
    // The extents are the half-widths of the box.
    groundBox.SetAsBox( kPhysicsScale * getWindowWidth(), kPhysicsScale * 10.0f );
    
    // Add the ground fixture to the ground body.
    groundBody->CreateFixture(&groundBox, 0.0f);

    gl::setMatricesWindow( getWindowSize() );
    gl::disable(GL_DEPTH_TEST);
    gl::disable(GL_CULL_FACE);
    gl::enable(GL_PROGRAM_POINT_SIZE_EXT);
    gl::enableAdditiveBlending();

    mParticleShader = gl::GlslProg( loadAsset("particle.glslv"), loadAsset("particle.glslf") );
    mParticleShader.bind();
    mParticleShader.uniform( "radius", mParticleSystem->GetRadius() / kPhysicsScale );
    mParticleShaderPosition = mParticleShader.getAttribLocation("position");
    mParticleShaderColor = mParticleShader.getAttribLocation("color");
}

void CircleEngineApp::mouseDrag( MouseEvent event )
{
    Vec2f pos = event.getPos();

    b2CircleShape shape;
    shape.m_p.Set( kPhysicsScale * pos.x, kPhysicsScale * pos.y );
    shape.m_radius = kPhysicsScale * 10.0f;
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
    // clear out the window with black
    gl::clear( Color( 0, 0, 0 ) );
    
    drawParticles();
}

void CircleEngineApp::drawParticles()
{
    int particleCount = mParticleSystem->GetParticleCount();
    const b2Vec2* positionBuffer = mParticleSystem->GetPositionBuffer();
    const b2ParticleColor* colorBuffer = mParticleSystem->GetColorBuffer();
    
    mParticleShader.bind();
    glPushMatrix();
    glScalef(1.0f/kPhysicsScale, 1.0f/kPhysicsScale, 1.0f/kPhysicsScale);
    glVertexAttribPointer(mParticleShaderPosition, 2, GL_FLOAT, GL_FALSE, 0, &positionBuffer[0].x);
    glVertexAttribPointer(mParticleShaderColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colorBuffer[0].r);
    glEnableVertexAttribArray(mParticleShaderPosition);
    glEnableVertexAttribArray(mParticleShaderColor);
    glDrawArrays(GL_POINTS, 0, particleCount);
    glDisableVertexAttribArray(mParticleShaderPosition);
    glDisableVertexAttribArray(mParticleShaderColor);
    glPopMatrix();
}

CINDER_APP_NATIVE( CircleEngineApp, RendererGl )
