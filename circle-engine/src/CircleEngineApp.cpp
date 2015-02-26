// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#include "cinder/app/AppNative.h"
#include "cinder/params/Params.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/svg/Svg.h"
#include "cinder/svg/SvgGl.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"
#include "Cinder-AppNap.h"
#include "CircleWorld.h"
#include "ParticleRender.h"
#include "FadecandyGL.h"
#include "MacMidiHotplug.h"
#include <OpenGL/OpenGL.h>
#include <mutex>
#include <condition_variable>

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
    void keyDown(KeyEvent event);

private:
    void drawObstacles();
    void drawFrontLayer();
    void drawSpinnerActivity();
    void drawSpinners();
    void drawForceGrid();
    void reloadColorTable();
    void deleteAllParticles();
    void clearColorCubes();
    void logCurrentSpinnerAngle();
    void seekBackward();
    void seekForward();
    
    static void physicsThreadFn(CircleEngineApp *self);
    void physicsLoop(ci::midi::Hub &midi);
    
    static void fadecandyThreadFn(CircleEngineApp *self);
    void fadecandyLoop();
    
    CGLContextObj       mMainGlContext;
    FadecandyGL         mFadecandy;
    MacMidiHotplug      mMidiHotplug;
    thread              mPhysicsThread;
    thread              mFadecandyThread;
    
    mutex               mPhysicsMutex;
    mutex               mPhysicsFrameDoneMutex;
    condition_variable  mPhysicsFrameDoneCond;
    bool                mPhysicsFrameDoneFlag;
    bool                mExiting;
    CircleWorld         mWorld;
    ParticleRender      mParticleRender;
    Rectf               mParticleRect;
    gl::VboMeshRef      mObstaclesVbo;
    gl::VboMeshRef      mFrontLayerVbo;
    gl::Fbo             mFrontLayerFbo;
    gl::Fbo             mFeedbackMaskFbo;

    int                 mSeekPending;
    mutex               mSeekMutex;
    uint64_t            mFrameCounter;
    Vec2i               mLastMousePos;  // For access from the FadecandyThread
    
    params::InterfaceGlRef      mParams;
    float                       mDisplayFps;
    float                       mPhysicsFps;
    float                       mFadecandyFps;
    float                       mTargetPhysicsFps;
    float                       mAmbientLight;
    unsigned                    mNumParticles;
    bool                        mDrawForceGrid;
    bool                        mDrawLedBuffer;
    bool                        mDrawLedModel;
    bool                        mDrawColorTable;
    int                         mDrawSpinnerColorCube;
    bool                        mDrawObstacles;
    bool                        mDrawParticles;
    bool                        mDrawFrontLayer;
    bool                        mSelectedSpinnerControlsAll;
    bool                        mDisableLedUpdates;
    bool                        mDrawSensorActivity;
    bool                        mMappingTestMode;
};

void CircleEngineApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize( 1280, 720 );
}

void CircleEngineApp::setup()
{
    cerr << "Begin setup" << endl;
    Cinder::AppNap::BeginActivity("CircleEngine LED rendering");

    mWorld.setup(svg::Doc::create(loadResource("world.svg")));
    cerr << "World loaded" << endl;

    mObstaclesVbo = gl::VboMesh::create(mWorld.mObstacles);
    mFrontLayerVbo = gl::VboMesh::create(mWorld.mFrontLayer);

    reloadColorTable();
    cerr << "Color table loaded" << endl;

    mFadecandy.setup(*this);
    mFadecandy.setModel(mWorld.mLedPoints);
    cerr << "LED model setup" << endl;
    
    float scale = 2;
    mParticleRect = Rectf(0, 0, getWindowWidth(), getWindowHeight());
    mParticleRender.setup( *this, getWindowWidth() / scale, getWindowHeight() / scale, 1.0f / scale / mWorld.kMetersPerPoint );
    mParticleRender.setForceGrid(mWorld.mForceGridSurface,
                                 Rectf(mWorld.mForceGridExtent.getUpperLeft() / mParticleRect.getSize(),
                                       mWorld.mForceGridExtent.getLowerRight() / mParticleRect.getSize()));
    cerr << "Particle renderer setup" << endl;

    mParams = params::InterfaceGl::create( getWindow(), "Engine parameters", toPixels(Vec2i(240, 680)) );
    mParams->minimize();
    
    mParams->addParam("Display FPS", &mDisplayFps, "readonly=true");
    mParams->addParam("Physics FPS", &mPhysicsFps, "readonly=true");
    mParams->addParam("Fadecandy FPS", &mFadecandyFps, "readonly=true");
    mParams->addParam("Target physics FPS", &mTargetPhysicsFps);
    mParams->addParam("# particles", &mNumParticles, "readonly=true");
    mParams->addParam("LED sampling radius", &mFadecandy.samplingRadius).min(0.f).max(500.f).step(0.1f);
    mParams->addSeparator();
    mParams->addButton("Reload color table", bind( &CircleEngineApp::reloadColorTable, this ), "key=c");
    mParams->addButton("Delete all particles", bind( &CircleEngineApp::deleteAllParticles, this ), "key=d");
    mParams->addParam("Color change rate", &mWorld.mColorChooser.mSpeed).min(0.f).max(100.f).step(0.1f);
    mParams->addButton("Seek backward", bind( &CircleEngineApp::seekBackward, this ), "key=,");
    mParams->addButton("Seek forward", bind( &CircleEngineApp::seekForward, this ), "key=.");
    mParams->addSeparator();
    mParams->addParam("Particle brightness", &mParticleRender.mBrightness).min(0.f).max(5.f).step(0.01f);
    mParams->addParam("Feedback gain", &mParticleRender.mFeedbackGain).min(0.f).max(100.f).step(0.001f);
    mParams->addParam("Feedback flow", &mParticleRender.mFeedbackFlow).min(-10.f).max(100.f).step(0.001f);
    mParams->addParam("Max particle rate", &mWorld.mMaxParticleRate);
    mParams->addParam("Max particle lifetime", &mWorld.mMaxParticleLifetime);
    mParams->addParam("Force grid strength", &mWorld.mForceGridStrength).min(0.f).max(100.f).step(0.01f);
    mParams->addSeparator();
    mParams->addParam("Draw force grid", &mDrawForceGrid, "key=1");
    mParams->addParam("Draw LED model", &mDrawLedModel, "key=2");
    mParams->addParam("Draw LED buffer", &mDrawLedBuffer, "key=3");
    mParams->addParam("Draw obstacles", &mDrawObstacles, "key=4");
    mParams->addParam("Draw particles", &mDrawParticles, "key=5");
    mParams->addParam("Draw front layer", &mDrawFrontLayer, "key=6");
    mParams->addParam("Draw color table", &mDrawColorTable, "key=7");
    mParams->addParam("Draw sensor activity", &mDrawSensorActivity, "key=8");
    mParams->addParam("Ambient light", &mAmbientLight).min(0.f).max(1.f).step(0.01f);
    mParams->addSeparator();
    mParams->addParam("Spin randomly", &mWorld.mMoveSpinnersRandomly, "key=r");
    mParams->addParam("Disable LED updates", &mDisableLedUpdates);
    mParams->addParam("Mapping test mode", &mMappingTestMode, "key=m");
    mParams->addParam("Spinner angle offset", &mWorld.mSpinnerOffset).min(-180).max(180).step(.1f);
    mParams->addParam("Spinner motor power", &mWorld.mSpinnerPower).min(0.f).max(100.f).step(.01f);
    mParams->addParam("Show color cube test", &mDrawSpinnerColorCube).min(-1).max(40).keyDecr("[").keyIncr("]");
    mParams->addParam("One spinner ctrl all", &mWorld.mOneSpinnerControlsAll);
    mParams->addButton("Clear all color cubes", bind( &CircleEngineApp::clearColorCubes, this ), "key=q");
    mParams->addButton("Log current spinner angle", bind( &CircleEngineApp::logCurrentSpinnerAngle, this ), "key=l");
    mParams->addParam("Log MIDI messages", &mWorld.mLogMidiMessages);
    
    gl::disable(GL_DEPTH_TEST);
    gl::disable(GL_CULL_FACE);
    
    mDrawLedBuffer = false;
    mDrawForceGrid = false;
    mDrawSpinnerColorCube = -1;
    mDrawObstacles = true;
    mDrawLedModel = false;
    mDrawParticles = true;
    mDrawFrontLayer = true;
    mDrawSensorActivity = false;
    mDrawColorTable = false;
    mExiting = false;
    mDisableLedUpdates = false;
    mPhysicsFrameDoneFlag = false;
    mMappingTestMode = false;
    mAmbientLight = 0.05f;
    mFrameCounter = 0;
    mTargetPhysicsFps = 120.0f;
    
    mMainGlContext = CGLGetCurrentContext();
    mPhysicsThread = thread(physicsThreadFn, this);
    mFadecandyThread = thread(fadecandyThreadFn, this);
    mMidiHotplug.start();
}

void CircleEngineApp::keyDown(KeyEvent event)
{
    if (event.getCode() == KeyEvent::KEY_TAB) {
        if (mParams->isMaximized())
            mParams->minimize();
        else
            mParams->maximize();
    }
}

void CircleEngineApp::reloadColorTable()
{
    // Do this with the lock held, since we're reallocating the image
    ci::ImageSourceRef table = loadImage(loadResource("forest-palette.png"));
    mPhysicsMutex.lock();
    mWorld.initColors(table);
    mPhysicsMutex.unlock();
}

void CircleEngineApp::deleteAllParticles()
{
    mPhysicsMutex.lock();
    for (unsigned i = 0 ; i < mWorld.mParticleSystem->GetParticleCount(); i++) {
        mWorld.mParticleSystem->DestroyParticle(i);
    }
    mPhysicsMutex.unlock();
}

void CircleEngineApp::clearColorCubes()
{
    mPhysicsMutex.lock();
    mWorld.clearColorCubes();
    mPhysicsMutex.unlock();
}

void CircleEngineApp::seekBackward()
{
    mSeekMutex.lock();
    mSeekPending--;
    mSeekMutex.unlock();
}

void CircleEngineApp::seekForward()
{
    mSeekMutex.lock();
    mSeekPending++;
    mSeekMutex.unlock();
}

void CircleEngineApp::physicsThreadFn(CircleEngineApp *self)
{
    midi::Hub midi;
    
    while (!self->mExiting) {
        self->physicsLoop(midi);
    }
}

void CircleEngineApp::physicsLoop(midi::Hub &midi)
{
    const unsigned kStepsPerMeasurement = 10;
    
    mSeekMutex.lock();
    int seekSteps = mSeekPending;
    mSeekPending = 0;
    mSeekMutex.unlock();
    
    mPhysicsMutex.lock();
    mWorld.mColorChooser.seek(seekSteps);
    ci::Timer stepTimer(true);

    for (unsigned i = kStepsPerMeasurement; i; i--) {
        mWorld.mColorChooser.update();
        mWorld.update(midi);
        if (mPhysicsFps > mTargetPhysicsFps) {
            mWorld.particleBurst();
        }
        mPhysicsFrameDoneMutex.lock();
        mPhysicsFrameDoneFlag = true;
        mPhysicsFrameDoneCond.notify_all();
        mPhysicsFrameDoneMutex.unlock();
    }
    
    mPhysicsFps = kStepsPerMeasurement / stepTimer.getSeconds();
    mPhysicsMutex.unlock();
}

void CircleEngineApp::fadecandyThreadFn(CircleEngineApp *self)
{
    // New OpenGL context with shared resources
    
    CGDirectDisplayID display = CGMainDisplayID();
    CGOpenGLDisplayMask myDisplayMask = CGDisplayIDToOpenGLDisplayMask(display);
    CGLPixelFormatAttribute attribs[] = {
        kCGLPFADisplayMask,
        (CGLPixelFormatAttribute)myDisplayMask,
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj pixelFormat = NULL;
    GLint numPixelFormats = 0;
    CGLContextObj myCGLContext = 0;
    CGLChoosePixelFormat(attribs, &pixelFormat, &numPixelFormats);
    CGLCreateContext(pixelFormat, self->mMainGlContext, &myCGLContext);
    CGLSetCurrentContext(myCGLContext);
    
    while (!self->mExiting) {
        self->fadecandyLoop();
    }
}

void CircleEngineApp::fadecandyLoop()
{
    const unsigned kStepsPerMeasurement = 10;
    
    ci::Timer stepTimer(true);
    
    for (unsigned i = kStepsPerMeasurement; i; i--) {
        // Wait for new data from the physics thread, if necessary
        std::unique_lock<std::mutex> lk(mPhysicsFrameDoneMutex);
        while (!mPhysicsFrameDoneFlag) {
            mPhysicsFrameDoneCond.wait(lk);
        }
        mPhysicsFrameDoneFlag = false;
            
        if (mFeedbackMaskFbo) {
            
            if (mMappingTestMode) {
                mParticleRender.renderSingle(mWorld.vecToBox(mLastMousePos), mFeedbackMaskFbo.getTexture());
            } else {
                mParticleRender.render(*mWorld.mParticleSystem, mFeedbackMaskFbo.getTexture());
            }
                
            if (mDisableLedUpdates) {
                // Without LED readback there are no natural sync points, so GPU interaction gets laggy
                glFinish();
            } else {
                mFadecandy.update(mParticleRender.getTexture(),
                                  Matrix33f::createScale( Vec2f(1.0f / mParticleRect.getWidth(),
                                                                1.0f / mParticleRect.getHeight()) ));
            }
        }
    }
    
    mFadecandyFps = kStepsPerMeasurement / stepTimer.getSeconds();
}

void CircleEngineApp::shutdown()
{
    mExiting = true;
    mPhysicsThread.join();
    mFadecandyThread.join();
    mMidiHotplug.stop();
}

void CircleEngineApp::update()
{
    mDisplayFps = getAverageFps();
    mNumParticles = mWorld.mParticleSystem->GetParticleCount();
    mLastMousePos = getMousePos() - getWindowPos();
}

void CircleEngineApp::draw()
{
    mFrameCounter++;

    // Pre-render feedback mask into an FBO from the SVG source
    // HACK: Re-render for the first N frames, possible GL state management bug or driver bug
    if (!mFeedbackMaskFbo) {
        mFeedbackMaskFbo = gl::Fbo(getWindowWidth(), getWindowHeight(), false);
    }
    if (mFrameCounter < 30) {
        mFeedbackMaskFbo.bindFramebuffer();
        SvgRendererGl rGl;
        gl::setViewport(Area(Vec2f(0,0), mFeedbackMaskFbo.getSize()));
        gl::setMatricesWindow(mFeedbackMaskFbo.getSize(), false);
        gl::clear();
        gl::color(1.0f, 1.0f, 1.0f);
        mWorld.findNode("feedback").render(rGl);
        mFeedbackMaskFbo.unbindFramebuffer();
    }

    gl::setViewport(Area(Vec2f(0,0), getWindowSize()));
    gl::setMatricesWindowPersp( getWindowSize() );
    gl::clear();

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
    
    if (mDrawFrontLayer) {
        if (mDrawObstacles) {
            drawSpinners();
        }
        drawFrontLayer();
    } else {
        if (mDrawObstacles) {
            drawObstacles();
            drawSpinners();
        }
    }
    
    if (mDrawSensorActivity) {
        drawSpinnerActivity();
    }

    if (mDrawLedModel) {
        mFadecandy.drawModel();
    }

    if (mDrawLedBuffer) {
        const gl::Texture& tex = mFadecandy.getFramebufferTexture();
        float scale = 4.0f;
        Vec2f topLeft(400, 10);
        gl::disableAlphaBlending();
        gl::color(1.0f, 1.0f, 1.0f, 1.0f);
        gl::draw(tex, Rectf(topLeft, topLeft + tex.getSize() * scale));
    }

    if (mDrawColorTable) {
        gl::pushModelView();
        gl::translate(getWindowWidth() * 0.75f, getWindowHeight() * 0.5f, 0.0f);
        float scale = getWindowHeight() * 0.66;
        gl::scale(scale, scale, scale);
        gl::translate(-0.5f, -0.5f, 0.0f);
        mWorld.mColorChooser.draw();
        gl::popModelView();
    }
    
    if (mDrawSpinnerColorCube >= 0 && mDrawSpinnerColorCube < mWorld.mSpinners.size()) {
        auto& spinner = mWorld.mSpinners[mDrawSpinnerColorCube];
        auto& cube = spinner.mColorCube;
        float s = getWindowWidth() * 0.25f;

        gl::pushModelView();
        gl::translate(getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f);
        gl::scale(Vec3f(s,s,s));
        gl::rotate(Vec3f(-10 - getMousePos().y * 0.06, 40 + getMousePos().x * 0.1, 0));
        gl::translate(-0.5f, -0.5f, -0.5f);
        cube.draw();
        gl::popModelView();

        gl::enableAlphaBlending();
        Vec2f cursor = Vec2f(300, getWindowHeight() * 0.75f);
        char str[128];

        snprintf(str, sizeof str, "Spinner #%d", mDrawSpinnerColorCube);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "%d points", (int)cube.getPoints().size());
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "Sensor angle: %.1f deg  (reliable = %d)", cube.getCurrentAngle() * 180.0 / M_PI, cube.isAngleReliable());
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "Target angle: %.1f deg", spinner.mTargetAngle * 180.0 / M_PI);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "RGB range: [%f, %f] [%f, %f] [%f, %f]",
                 cube.getRangeRGB().getMin().x, cube.getRangeRGB().getMax().x,
                 cube.getRangeRGB().getMin().y, cube.getRangeRGB().getMax().y,
                 cube.getRangeRGB().getMin().z, cube.getRangeRGB().getMax().z);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "RGB size: %f, %f %f",
                 cube.getRangeRGB().getSize().x,
                 cube.getRangeRGB().getSize().y,
                 cube.getRangeRGB().getSize().z);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "XYZ range: [%f, %f] [%f, %f] [%f, %f]",
                 cube.getRangeXYZ().getMin().x, cube.getRangeXYZ().getMax().x,
                 cube.getRangeXYZ().getMin().y, cube.getRangeXYZ().getMax().y,
                 cube.getRangeXYZ().getMin().z, cube.getRangeXYZ().getMax().z);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "XYZ size: %f, %f %f",
                 cube.getRangeXYZ().getSize().x,
                 cube.getRangeXYZ().getSize().y,
                 cube.getRangeXYZ().getSize().z);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "XY size: %f", cube.getRangeXYZ().getSize().xy().length());
        gl::drawString(str, cursor);
        cursor.y += 15.0f;

        snprintf(str, sizeof str, "XY / Z ratio: %f", cube.getRangeXYZ().getSize().xy().length() / cube.getRangeXYZ().getSize().z);
        gl::drawString(str, cursor);
        cursor.y += 15.0f;
    }
    
    mParams->draw();
}

void CircleEngineApp::drawObstacles()
{
    gl::disableAlphaBlending();
    gl::color(0.33f, 0.33f, 0.33f);
    gl::draw(mObstaclesVbo);

    gl::enableWireframe();
    gl::lineWidth(0.5f);
    gl::color(0.5f, 0.5f, 0.5f);
    gl::draw(mObstaclesVbo);
    gl::disableWireframe();
}

void CircleEngineApp::drawFrontLayer()
{
    if (!mFrontLayerFbo) {
        Vec2f size = getWindowSize();
        float oversample = 2;
        mFrontLayerFbo = gl::Fbo(size.x * oversample, size.y * oversample, true);
        mFrontLayerFbo.bindFramebuffer();
        
        gl::setViewport(Area(Vec2f(0.0f, 0.0f), mFrontLayerFbo.getSize()));
        gl::setMatricesWindow(size, false);
        gl::clear(ColorA(0.0f, 0.0f, 0.0f, 0.0f));
        
        gl::enableAlphaBlending();
        gl::color(1.0f, 1.0f, 1.0f);
        gl::draw(mFrontLayerVbo);
        
        mFrontLayerFbo.unbindFramebuffer();
    }
    
    gl::enableAlphaBlending();
    gl::color(mAmbientLight, mAmbientLight, mAmbientLight);
    gl::draw(mFrontLayerFbo.getTexture(), Rectf(Vec2f(0,0), getWindowSize()));
}

void CircleEngineApp::drawSpinners()
{
    gl::enableAlphaBlending();
    gl::color(mAmbientLight, mAmbientLight, mAmbientLight);
    
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
                      
void CircleEngineApp::drawSpinnerActivity()
{
    gl::enableAdditiveBlending();
    
    for (unsigned i = 0; i < mWorld.mSpinners.size(); i++) {
        auto& spinner = mWorld.mSpinners[i];
        b2Body *body = spinner.mBody;
        TriMesh2d &mesh = spinner.mMesh;
        
        float a = 1.0f / (1.0f + 5.0f * spinner.mColorCube.getTimeSinceLastPoint());
        gl::color(0.5f * a, 2.0f * a, 0.5f * a, a);

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

void CircleEngineApp::logCurrentSpinnerAngle()
{
    if (mDrawSpinnerColorCube >= 0 && mDrawSpinnerColorCube < mWorld.mSpinners.size()) {
        auto& cube = mWorld.mSpinners[mDrawSpinnerColorCube].mColorCube;
        printf("%f\n", cube.getCurrentAngle());
    } else {
        printf("No spinner selected in color cube debug view\n");
    }
}

CINDER_APP_NATIVE( CircleEngineApp, RendererGl )
