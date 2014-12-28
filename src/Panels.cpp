#include "Panels.h"

using namespace ci;
using namespace std;


const float kPanelSizeX = 2/16.0;
const float kPanelSizeY = 4/16.0;


Panels::Panels(Dots &dots) :
    mDots(dots),
    mEdgeK(0.01),
    mEdgeMargin(0.01)
{
    reset();
}


void Panels::reset()
{
    mOutlines.clear();
 
    for (float x = 0; x < 1.0; x += kPanelSizeX) {
        for (float y = 0; y < 0.5; y += kPanelSizeY) {
            
            mOutlines.push_back(PolyLine2f());
            mOutlines.back().push_back(Vec2f(x, y));
            mOutlines.back().push_back(Vec2f(x + kPanelSizeX, y));
            mOutlines.back().push_back(Vec2f(x + kPanelSizeX, y + kPanelSizeY));
            mOutlines.back().push_back(Vec2f(x, y + kPanelSizeY));
            mOutlines.back().setClosed();

        }
    }
}


void Panels::update()
{
    // Vertical boundary forces
    
    float k = mEdgeK;
    float margin = mEdgeMargin;

    for (float x = kPanelSizeX; x < 1.0; x += kPanelSizeX) {
        for (unsigned dot = 0; dot < mDots.mDotPoints.size(); dot++) {
            Vec2f &point = mDots.mDotPoints[dot];
            float size = mDots.getDotSize(dot);
            float t = point.x - x;
            
            if (t > 0.0f && t < size + margin) {
                point.x += k * (size + margin - t);
            }
            if (t < 0.0f && t > -size - margin) {
                point.x -= k * (t + size + margin);
            }
        }
    }

    // Horizontal boundary forces
    
    for (float y = kPanelSizeY; y < 0.5; y += kPanelSizeY) {
        for (unsigned dot = 0; dot < mDots.mDotPoints.size(); dot++) {
            Vec2f &point = mDots.mDotPoints[dot];
            float size = mDots.getDotSize(dot);
            float t = point.y - y;
                       
            if (t > 0.0f && t < size + margin) {
                point.y += k * (size + margin - t);
            }
            if (t < 0.0f && t > -size - margin) {
                point.y -= k * (t + size + margin);
            }
        }
    }
}


void Panels::draw()
{
    gl::enableAlphaBlending();
    
    for (unsigned panel = 0; panel < mOutlines.size(); panel++) {
        PolyLine2f &outline = mOutlines[panel];
        
        gl::color(0.1, 0.5, 0.1, 0.5);
        gl::draw(outline);
    }
    
    gl::disableAlphaBlending();
}
