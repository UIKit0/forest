#include "cinder/app/AppBasic.h"
#include "cinder/BSpline.h"
#include "cinder/cairo/Cairo.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "strands.h"
#include <vector>

using namespace ci;
using namespace ci::app;
using namespace std;

class forestApp : public AppBasic {
  public:
    void prepareSettings( Settings *settings );
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();

    cinder::Rand rand;
    vector<Strand> strands;
};

void forestApp::prepareSettings( Settings *settings )
{
    settings->setWindowSize(1280, 720);
    settings->setFrameRate(60.0);
}

void forestApp::setup()
{

    for (int i = 0; i < 100; i++) {
        strands.push_back(Strand());
        strands.back().seed(Vec2d( rand.nextFloat(), 0 ));
    }
}

void forestApp::mouseDown( MouseEvent event )
{
}

void forestApp::draw()
{
    cairo::Context ctx( cairo::createWindowSurface() );
    ctx.setSourceRgb( 0.9f, 0.9f, 0.9f );
    ctx.paint();

    float wallWidth = getWindowWidth() * 0.75;
    float wallHeightRatio = 0.5;
    float wallHeight = wallWidth * wallHeightRatio;
    Vec2f wallSize(wallWidth, wallHeight);
    Vec2f wallTopLeft = getWindowCenter() - wallSize/2;

    ctx.rectangle(wallTopLeft.x, wallTopLeft.y, wallWidth, wallHeight);
    ctx.setSourceRgb( 1.0f, 1.0f, 1.0f );
    ctx.fill();
    
    for (int s = 0; s < strands.size(); s++) {
        Strand &strand = strands[s];
        
        for (int p = 0; p < strand.points.size(); p++) {
            Vec2f pos( wallTopLeft.x + wallWidth * strand.points[p].x,
                      wallTopLeft.y + wallWidth * (wallHeightRatio - strand.points[p].y) );
            if (p == 0) {
                ctx.moveTo(pos);
            } else {
                ctx.lineTo(pos);
            }
        }
    
        ctx.setLineCap(cairo::LINE_CAP_ROUND);
        ctx.setLineWidth( 1.0 );
        ctx.setSourceRgb( 0.0f, 0.0f, 0.0f );
        ctx.stroke();
    }
}

void forestApp::update()
{
    for (int growStep = 0; growStep < 10; growStep++) {
        
        double k = 0.1;
        
        for (int i = 0; i < strands.size(); i++) {

            for (int step = 0; step < 20; step++) {
                strands[i].springForce(k, 0.005);
                strands[i].straightenForce(k);
            }
        }

        Strand &randStrand = strands[rand.nextInt(strands.size())];
        if (randStrand.points.size() < 80) {
            randStrand.grow(rand, Vec2f(0, 0.85));
        }
    }
}

CINDER_APP_BASIC( forestApp, Renderer2d )
