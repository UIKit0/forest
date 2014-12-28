#pragma once

#include "Dots.h"
#include "cinder/PolyLine.h"
#include "cinder/Json.h"
#include <vector>


// Breaks up the entire box into small panels that can be manufactured separately
class Panels
{
public:
    Panels(Dots &dots);
    
    ci::JsonTree serialize();
    
    void reset();
    void update();
    void draw();
  
    Dots& mDots;
    
    float mEdgeK;
    float mEdgeMargin;

    std::vector<ci::PolyLine2f>   mOutlines;
};
