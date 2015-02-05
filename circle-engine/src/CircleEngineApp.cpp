// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/app/AppNative.h"
#include "cinder/params/Params.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/svg/Svg.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"
#include "Cinder-AppNap.h"
#include "CircleWorld.h"
#include "ParticleRender.h"
#include "FadecandyGL.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CircleEngineApp : public AppNative {
public:
    void prepareSettings( Settings *settings );
    void setup();
    void shutdown();
    void update();
    void draw();

private:
    void drawObstacles();
    void drawSpinners();
    void drawForceGrid();
    void reloadColorTable();
    void clearColorCubes();

    static void physicsThreadFn(CircleEngineApp *self);
    
    FadecandyGL         mFadecandy;
    thread              mPhysicsThread;
    mutex               mPhysicsMutex;
    bool                mExiting;
    CircleWorld         mWorld;
    ParticleRender      mParticleRender;
    Rectf               mParticleRect;
    gl::VboMeshRef      mObstaclesVbo;
    
    params::InterfaceGlRef      mParams;
    float                       mAverageFps;
    float                       mPhysicsHz;
    unsigned                    mNumParticles;
    bool                        mDrawForceGrid;
    bool                        mDrawLedBuffer;
    bool                        mDrawLedModel;
    int                         mDrawSpinnerColorCube;
    bool                        mDrawObstacles;
    bool                        mDrawParticles;
};

void CircleEngineApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1280, 720 );
    settings->disableFrameRate();
}

void CircleEngineApp::setup()
{
    Cinder::AppNap::BeginActivity("CircleEngine LED rendering");

    mWorld.setup(svg::Doc::create(loadAsset("world.svg")));
    mObstaclesVbo = gl::VboMesh::create(mWorld.mObstacles);
    reloadColorTable();

    mFadecandy.setup(*this);
    mFadecandy.setModel(mWorld.mLedPoints);
    
    float scale = 2;
    mParticleRect = Rectf(0, 0, getWindowWidth(), getWindowHeight());
    mParticleRender.setup( *this, getWindowWidth() / scale, getWindowHeight() / scale, 1.0f / scale / mWorld.kMetersPerPoint );
 
    mParams = params::InterfaceGl::create( getWindow(), "Engine parameters", toPixels(Vec2i(240, 600)) );
    
    mParams->addParam("FPS", &mAverageFps, "readonly=true");
    mParams->addParam("Physics Hz", &mPhysicsHz, "readonly=true");
    mParams->addParam("# particles", &mNumParticles, "readonly=true");
    mParams->addSeparator();
    mParams->addParam("Current table row", &mWorld.mCurrentTableRow, "readonly=true");
    mParams->addButton("Reload color table", bind( &CircleEngineApp::reloadColorTable, this ), "key=c");
    mParams->addSeparator();
    mParams->addParam("Particle brightness", &mParticleRender.mBrightness).min(0.f).max(5.f).step(0.01f);
    mParams->addParam("Particle rate", &mWorld.mNewParticleRate);
    mParams->addParam("Particle lifetime", &mWorld.mNewParticleLifetime);
    mParams->addParam("LED sampling radius", &mFadecandy.samplingRadius).min(0.f).max(500.f).step(0.1f);
    mParams->addSeparator();
    mParams->addParam("Draw force grid", &mDrawForceGrid);
    mParams->addParam("Draw LED model", &mDrawLedModel);
    mParams->addParam("Draw LED buffer", &mDrawLedBuffer);
    mParams->addParam("Draw obstacles", &mDrawObstacles);
    mParams->addParam("Draw particles", &mDrawParticles);
    mParams->addSeparator();
    mParams->addParam("Spin randomly", &mWorld.mMoveSpinnersRandomly);
    mParams->addParam("Show color cube test", &mDrawSpinnerColorCube).min(-1).max(40).keyDecr("[").keyIncr("]");
    mParams->addButton("Clear all color cubes", bind( &CircleEngineApp::clearColorCubes, this ), "key=q");
    
    gl::disableVerticalSync();
    gl::disable(GL_DEPTH_TEST);
    gl::disable(GL_CULL_FACE);
    
    mDrawLedBuffer = false;
    mDrawForceGrid = false;
    mDrawSpinnerColorCube = -1;
    mDrawObstacles = true;
    mDrawLedModel = false;
    mDrawParticles = true;
    mExiting = false;

    mPhysicsThread = thread(physicsThreadFn, this);
}

void CircleEngineApp::reloadColorTable()
{
    // Do this with the lock held, since we're reallocating the image
    mPhysicsMutex.lock();
    mWorld.initColors(loadImage(loadAsset("colors.png")));
    mPhysicsMutex.unlock();
}

void CircleEngineApp::clearColorCubes()
{
    mPhysicsMutex.lock();
    mWorld.clearColorCubes();
    mPhysicsMutex.unlock();
}

void CircleEngineApp::physicsThreadFn(CircleEngineApp *self)
{
    const unsigned kStepsPerMeasurement = 10;
    ci::midi::Hub midi;
    
    while (!self->mExiting) {
        self->mPhysicsMutex.lock();
        ci::Timer stepTimer(true);
        for (unsigned i = kStepsPerMeasurement; i; i--) {
            self->mWorld.update(midi);
        }
        self->mPhysicsHz = kStepsPerMeasurement / stepTimer.getSeconds();
        self->mPhysicsMutex.unlock();
    }
}

void CircleEngineApp::shutdown()
{
    mExiting = true;
    mPhysicsThread.join();
    Cinder::AppNap::EndActivity();
}

void CircleEngineApp::update()
{

    mAverageFps = getAverageFps();
    mNumParticles = mWorld.mParticleSystem->GetParticleCount();
}

void CircleEngineApp::draw()
{
    mParticleRender.render(*mWorld.mParticleSystem);

    gl::setViewport(Area(Vec2f(0,0), getWindowSize()));
    gl::setMatricesWindowPersp( getWindowSize() );
    gl::clear(Color( 0, 0, 0 ));

    if (mDrawParticles) {
        gl::enable(GL_TEXTURE_2D);
        mParticleRender.getTexture().bind();
        gl::color(1,1,1,1);
        gl::drawSolidRect(mParticleRect);
        gl::disable(GL_TEXTURE_2D);
    }
        
    if (mDrawForceGrid) {
        drawForceGrid();
    }

    if (mDrawObstacles) {
        drawObstacles();
        drawSpinners();
    }
    
    if (mDrawLedModel) {
        mFadecandy.drawModel();
    }
        
    if (mDrawLedBuffer) {
        const gl::Texture& tex = mFadecandy.getFramebufferTexture();
        float scale = 4.0;
        Vec2f topLeft(400, 10);
        gl::disableAlphaBlending();
        gl::color(1.0f, 1.0f, 1.0f, 1.0f);
        gl::draw(tex, Rectf(topLeft, topLeft + tex.getSize() * scale));
    }

    if (mDrawSpinnerColorCube >= 0 && mDrawSpinnerColorCube <= mWorld.mSpinners.size()) {
        float s = getWindowWidth() * 0.25f;
        gl::pushModelView();
        gl::translate(getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f);
        gl::scale(Vec3f(s,s,s));
        gl::rotate(Vec3f(-10 - getMousePos().y * 0.06, 40 + getMousePos().x * 0.1, 0));
        gl::translate(-0.5f, -0.5f, -0.5f);
        mWorld.mSpinners[mDrawSpinnerColorCube].mColorCube.draw();
        gl::popModelView();
    }
    
    mParams->draw();
    
    // Update LEDs from contents of the particle rendering FBO.
    // Only runs if the simulation has produced a new step; if the physics
    // sim is running slower, rely on FC to interpolate between its frames.
    if (mWorld.mUpdatedSinceLastDraw) {
        mWorld.mUpdatedSinceLastDraw = false;
        mFadecandy.update(mParticleRender.getTexture(),
                Matrix33f::createScale( Vec2f(1.0f / mParticleRect.getWidth(),
                                              1.0f / mParticleRect.getHeight()) ));
    }
}

void CircleEngineApp::drawObstacles()
{
    gl::disableAlphaBlending();
    gl::color(0.33f, 0.33f, 0.33f);
    gl::draw(mObstaclesVbo);

    gl::enableWireframe();
    gl::color(0.5f, 0.5f, 0.5f);
    gl::draw(mObstaclesVbo);
    gl::disableWireframe();
}

void CircleEngineApp::drawSpinners()
{
    gl::disableAlphaBlending();
    gl::color(0.33f, 0.33f, 0.33f);
    
    for (unsigned i = 0; i < mWorld.mSpinners.size(); i++) {
        b2Body *body = mWorld.mSpinners[i].mBody;
        TriMesh2d &mesh = mWorld.mSpinners[i].mMesh;

        gl::pushMatrices();
        gl::translate(mWorld.vecFromBox(body->GetPosition()));
        gl::rotate(body->GetAngle() * 180 / M_PI);
        gl::draw(mesh);
        gl::popMatrices();
    }
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
