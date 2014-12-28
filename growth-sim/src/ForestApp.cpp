#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"
#include "cinder/Thread.h"
#include "Strands.h"
#include "Dots.h"
#include "Panels.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class ForestApp : public AppBasic {
  public:
    ForestApp();
    ~ForestApp();

    void prepareSettings( Settings *settings );
	void setup();
    void draw();
    void update();

    void resetButton();
    void stopButton();
    
    void simThreadFn();

    params::InterfaceGlRef      mParams;
    cinder::Rand                mRand;
    StrandBox                   mStrandBox;
    Dots                        mDots;
    Panels                      mPanels;
    
    mutex                       mSimMutex;
    shared_ptr<thread>          mSimThread;
    bool                        mSimRunning;
    
    bool                        mShowMatrix;
};


ForestApp::ForestApp() :
    mDots(mStrandBox),
    mPanels(mDots),
    mSimRunning(false),
    mShowMatrix(false)
{}


void ForestApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize(1280, 720);
}


void ForestApp::setup()
{
    mParams = params::InterfaceGl::create( getWindow(), "Forest parameters", toPixels(Vec2i(240, 600)) );

    mParams->addButton("Reset", bind( &ForestApp::resetButton, this ), "key=r");
    mParams->addButton("Stop sim", bind( &ForestApp::stopButton, this ), "key=s");
    mParams->addParam("Sim step #", &mStrandBox.mSimulationStep, "readonly=true");
    
    mParams->addSeparator();

    mParams->addParam( "Number of seeds", &mStrandBox.mNumSeeds).min(0).max(16).step(1);
    mParams->addParam( "Number of strands", &mStrandBox.mNumStrands).min(0).max(1000).step(1);
    mParams->addParam( "Number of dots", &mDots.mNumDots).min(0).max(1000).step(1);
    mParams->addParam( "Strand length", &mStrandBox.mStrandLength).min(1).max(1000).step(1);
    mParams->addParam( "Growth probability", &mStrandBox.mGrowthProbability).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Growth dir Y", &mStrandBox.mGrowthDirection.y).min(-1.f).max(10.f).step(0.01f);

    mParams->addSeparator();

    mParams->addParam( "Spring length", &mStrandBox.mSpringLength).min(0.0001f).max(0.01f).step(0.0001f);
    mParams->addParam( "Spring iters", &mStrandBox.mSpringIterations).min(0).max(10000).step(10);
    mParams->addParam( "Spring K", &mStrandBox.mSpringK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Straighten K", &mStrandBox.mStraightenK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Smooth K", &mStrandBox.mSmoothK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Align K", &mStrandBox.mAlignmentK).min(0.f).max(4.0f).step(0.001f);

    mParams->addSeparator();

    mParams->addParam( "Smallest dot", &mDots.mSmallestDotSize).min(0.f).max(0.2f).step(0.001f);
    mParams->addParam( "Largest dot", &mDots.mLargestDotSize).min(0.f).max(0.2f).step(0.001f);
    mParams->addParam( "Dot gravity", &mDots.mDotGravity).min(0.f).max(20.0f).step(0.001f);
    mParams->addParam( "Dot spacing", &mDots.mDotSpacing).min(0.f).max(20.0f).step(0.001f);

    mParams->addSeparator();
    
    mParams->addParam( "Panel edge K", &mPanels.mEdgeK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Panel edge margin", &mPanels.mEdgeMargin).min(0.f).max(1.0f).step(0.001f);

    mParams->addSeparator();

    mParams->addParam( "Show affinity matrix", &mShowMatrix, "key=m");
    mParams->addParam( "Max strands per dot", &mDots.mDotMaxStrands).min(0).max(100).step(1);
    mParams->addParam( "Affinity repel K", &mDots.mRepelK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Affinity retain K", &mDots.mRetainK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Affinity decay K", &mDots.mDecayK).min(0.f).max(1.0f).step(0.001f);
    mParams->addParam( "Force attract K", &mDots.mAttractK).min(0.f).max(1.0f).step(0.001f);
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
    if (mSimThread && !mSimRunning) {
        // Our sim thread finished; discard it so we'll create a new one in update()
        mSimThread->join();
        mSimThread = 0;
    }

    mSimMutex.lock();
    mStrandBox.reset();
    mDots.reset();
    mSimMutex.unlock();
}


void ForestApp::stopButton()
{
    // Exits the simulation thread at the next chance we get
    mSimRunning = false;
}


void ForestApp::draw()
{
    gl::clear( Color::gray(0.9f) );

    float wallWidth = getWindowWidth() * 0.75;
    float wallHeight = wallWidth / mStrandBox.mBorderRect.getAspectRatio();
    Vec2f wallSize(wallWidth, wallHeight);
    Vec2f wallTopLeft = getWindowCenter() - wallSize/2;

    // Wall coordinate system, Y=0 at bottom with +Y in growth direction
    gl::pushModelView();
    gl::translate(wallTopLeft.x, wallTopLeft.y + wallHeight);
    gl::scale(wallWidth, -wallWidth);

    // Drawing should be safe without acquiring the mutex (performance)
    mStrandBox.draw();
    mDots.draw();
    mPanels.draw();

    gl::popModelView();
    
    if (mShowMatrix) {
        mDots.drawAffinityMatrix();
    }
    mParams->draw();
}


void ForestApp::simThreadFn()
{
    while (mSimRunning) {
        ci::ThreadSetup ciThread;
        
        mSimMutex.lock();
        mStrandBox.update();
        mDots.update();
        mPanels.update();
        mSimMutex.unlock();
    }
}


ForestApp::~ForestApp()
{
    mSimRunning = false;
    mSimThread->join();
}


CINDER_APP_BASIC( ForestApp, RendererGl )
