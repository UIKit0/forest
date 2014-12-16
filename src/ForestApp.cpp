#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"
#include "Strands.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class ForestApp : public AppBasic {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void update();
	void draw();
    void resetButton();

    params::InterfaceGlRef      mParams;
    cinder::Rand                mRand;
    StrandBox                   mStrandBox;
};

void ForestApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize(1280, 720);
    settings->setFrameRate(60.0);
}

void ForestApp::setup()
{
    mParams = params::InterfaceGl::create( getWindow(), "Forest parameters", toPixels(Vec2i(220, 400)) );

    mParams->addButton("Reset", std::bind( &ForestApp::resetButton, this ), "key=R");
    mParams->addParam( "Number of strands", &mStrandBox.mNumStrands).min(0).max(1000).step(1);
    mParams->addParam( "Strand length", &mStrandBox.mStrandLength).min(1).max(1000).step(1);
    mParams->addParam( "Growth probability", &mStrandBox.mGrowthProbability).min(0.f).max(1.f).step(0.001f);
    mParams->addParam( "Growth dir Y", &mStrandBox.mGrowthDirection.y).min(-1.f).max(1.f).step(0.01f);

    mParams->addParam( "Spring length", &mStrandBox.mSpringLength).min(0.001f).max(0.01f).step(0.001f);
    mParams->addParam( "Spring iters", &mStrandBox.mSpringIterations).min(0).max(10000).step(10);
    mParams->addParam( "Spring K", &mStrandBox.mSpringK).min(0.001f).max(1.0f).step(0.001f);
    mParams->addParam( "Straighten K", &mStrandBox.mStraightenK).min(0.001f).max(1.0f).step(0.001f);
}

void ForestApp::resetButton()
{
    mStrandBox.reset();
}

void ForestApp::draw()
{
    gl::clear( Color::gray(0.9f) );

    float wallWidth = getWindowWidth() * 0.75;
    float wallHeightRatio = 0.5;
    float wallHeight = wallWidth * wallHeightRatio;
    Vec2f wallSize(wallWidth, wallHeight);
    Vec2f wallTopLeft = getWindowCenter() - wallSize/2;

    // Wall coordinate system, Y=0 at bottom with +Y in growth direction
    gl::pushModelView();
    gl::translate(wallTopLeft.x, wallTopLeft.y + wallHeight);
    gl::scale(wallWidth, -wallWidth);

    // Background
    gl::color( Color::gray(1.0f) );
    gl::drawSolidRect( Rectf( 0.0f, 0.0f, 1.0f, wallHeightRatio ));

    mStrandBox.draw();

    gl::popModelView();
    
    mParams->draw();
}

void ForestApp::update()
{
    mStrandBox.update();
}

CINDER_APP_BASIC( ForestApp, RendererGl )
