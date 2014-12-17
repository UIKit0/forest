#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"
#include "cinder/Thread.h"
#include "Strands.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class ForestApp : public AppBasic {
  public:
    ~ForestApp();

    void prepareSettings( Settings *settings );
	void setup();
    void draw();
    void update();
    void resetButton();
    
    void simThreadFn();

    params::InterfaceGlRef      mParams;
    cinder::Rand                mRand;
    StrandBox                   mStrandBox;

    mutex                       mSimMutex;
    shared_ptr<thread>          mSimThread;
    bool                        mSimRunning;
};

void ForestApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize(1280, 720);
}

void ForestApp::setup()
{
    mParams = params::InterfaceGl::create( getWindow(), "Forest parameters", toPixels(Vec2i(220, 400)) );

    mParams->addButton("Reset", bind( &ForestApp::resetButton, this ), "key=r");
    mParams->addParam( "Number of strands", &mStrandBox.mNumStrands).min(0).max(1000).step(1);
    mParams->addParam( "Strand length", &mStrandBox.mStrandLength).min(1).max(1000).step(1);
    mParams->addParam( "Growth probability", &mStrandBox.mGrowthProbability).min(0.f).max(1.0f).step(0.01f);
    mParams->addParam( "Growth dir Y", &mStrandBox.mGrowthDirection.y).min(-1.f).max(1.f).step(0.01f);

    mParams->addParam( "Spring length", &mStrandBox.mSpringLength).min(0.0001f).max(0.01f).step(0.0001f);
    mParams->addParam( "Spring iters", &mStrandBox.mSpringIterations).min(0).max(10000).step(10);
    mParams->addParam( "Spring K", &mStrandBox.mSpringK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Straighten K", &mStrandBox.mStraightenK).min(0.f).max(1.0f).step(0.001f);

    mParams->addParam( "Align R min", &mStrandBox.mAlignmentRadiusMin).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Align R max", &mStrandBox.mAlignmentRadiusMax).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Align K", &mStrandBox.mAlignmentK).min(0.f).max(1.0f).step(0.001f);
}

void ForestApp::update()
{
    // Start updating in the background
    if (!mSimThread) {
        mSimRunning = true;
        mSimThread = make_shared<thread>(bind( &ForestApp::simThreadFn, this ));
    }
}

void ForestApp::resetButton()
{
    mSimMutex.lock();
    mStrandBox.reset();
    mSimMutex.unlock();
}

void ForestApp::draw()
{
    gl::clear( Color::gray(0.9f) );

    float wallWidth = getWindowWidth() * 0.75;
    float wallHeight = wallWidth * mStrandBox.mRect.getHeight() / mStrandBox.mRect.getWidth();
    Vec2f wallSize(wallWidth, wallHeight);
    Vec2f wallTopLeft = getWindowCenter() - wallSize/2;

    // Wall coordinate system, Y=0 at bottom with +Y in growth direction
    gl::pushModelView();
    gl::translate(wallTopLeft.x, wallTopLeft.y + wallHeight);
    gl::scale(wallWidth, -wallWidth);

    // Drawing should be safe without acquiring the mutex (performance)
    mStrandBox.draw();
    
    gl::popModelView();
    
    mParams->draw();
}

void ForestApp::simThreadFn()
{
    while (mSimRunning) {
        ci::ThreadSetup ciThread;
        
        mSimMutex.lock();
        mStrandBox.update();
        mSimMutex.unlock();
    }
}

ForestApp::~ForestApp()
{
    mSimRunning = false;
    mSimThread->join();
}


CINDER_APP_BASIC( ForestApp, RendererGl )
