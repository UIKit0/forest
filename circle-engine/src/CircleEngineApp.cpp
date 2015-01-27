// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/app/AppNative.h"
#include "cinder/params/Params.h"
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
    void update();
    void draw();

    void drawParticles();
    void drawObstacles();
    void drawSpinners();
    void drawForceGrid();
    
    CircleWorld         mWorld;

    gl::GlslProg        mParticleShader;
    GLuint              mParticleShaderPosition;
    GLuint              mParticleShaderColor;

    params::InterfaceGlRef      mParams;
    float                       mAverageFps;
    unsigned                    mNumParticles;
};

void CircleEngineApp::prepareSettings( Settings *settings )
{
    settings->setFrameRate( 60.0f );
    settings->setWindowSize( 1280, 720 );
}

void CircleEngineApp::setup()
{
    mWorld.setup(svg::Doc::create(loadAsset("world.svg")),
                 loadImage(loadAsset("colors.png")));

    mParams = params::InterfaceGl::create( getWindow(), "Engine parameters", toPixels(Vec2i(240, 600)) );
    
    mParams->addParam("FPS", &mAverageFps, "readonly=true");
    mParams->addParam("# particles", &mNumParticles, "readonly=true");

    gl::disableVerticalSync();
    
    mParticleShader = gl::GlslProg( loadAsset("particle.glslv"), loadAsset("particle.glslf") );
    mParticleShader.bind();
    mParticleShaderPosition = mParticleShader.getAttribLocation("position");
    mParticleShaderColor = mParticleShader.getAttribLocation("color");
}

void CircleEngineApp::update()
{
    mWorld.update();

    mAverageFps = getAverageFps();
    mNumParticles = mWorld.mParticleSystem->GetParticleCount();
}

void CircleEngineApp::draw()
{
    gl::setMatricesWindow( getWindowSize() );
    gl::disable(GL_DEPTH_TEST);
    gl::disable(GL_CULL_FACE);

    gl::clear( Color( 0, 0, 0 ) );
    
    drawParticles();
    drawForceGrid();
    drawObstacles();
    drawSpinners();
    
    mParams->draw();
}

void CircleEngineApp::drawObstacles()
{
    gl::disableAlphaBlending();
    gl::color(0.33f, 0.33f, 0.33f);
    gl::draw(mWorld.mObstacles);
    gl::enableWireframe();
    gl::color(0.5f, 0.5f, 0.5f);
    gl::draw(mWorld.mObstacles);
    gl::disableWireframe();
}

void CircleEngineApp::drawSpinners()
{
    gl::disableAlphaBlending();
    gl::color(0.33f, 0.33f, 0.33f);
    
    for (unsigned i = 0; i < mWorld.mSpinnerBodies.size(); i++) {
        b2Body *body = mWorld.mSpinnerBodies[i];
        TriMesh2d &mesh = mWorld.mSpinnerMeshes[i];

        gl::pushMatrices();
        gl::translate(mWorld.vecFromBox(body->GetPosition()));
        gl::rotate(body->GetAngle() * 180 / M_PI);
        gl::draw(mesh);
        gl::popMatrices();
    }

    gl::enableWireframe();
    gl::color(0.5f, 0.5f, 0.5f);

    gl::draw(mWorld.mObstacles);
    
    gl::disableWireframe();
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

void CircleEngineApp::drawForceGrid()
{
    gl::color(1.0f, 1.0f, 1.0f, 0.25f);
    gl::enableAlphaBlending();
    for (unsigned idx = 0; idx < mWorld.mForceGrid.size(); idx++) {
        Vec2f pos(idx % mWorld.mForceGridWidth, idx / mWorld.mForceGridWidth);
        Vec2f force = mWorld.mForceGrid[idx];
        pos = mWorld.mForceGridExtent.getUpperLeft() + pos * mWorld.mForceGridResolution;
        gl::drawLine(pos, pos + force * 0.05);
    }
}

CINDER_APP_NATIVE( CircleEngineApp, RendererGl )
