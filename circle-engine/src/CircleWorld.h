// Circle Engine (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "cinder/svg/Svg.h"
#include "cinder/Exception.h"
#include "cinder/Triangulate.h"
#include "cinder/TriMesh.h"
#include "Box2D/Box2D.h"
#include <string>

class CircleWorld {
public:
    void setup(ci::svg::DocRef doc);
    
    const ci::svg::Node& findNode(const std::string &name);
    ci::Shape2d findShape(const std::string &name);
    ci::Vec2f findMetric(const std::string &name);

    b2Vec2 vecToBox(ci::Vec2f v);

    const float kMetersPerPoint         = 10.0;
    const float kMinTriangleArea        = 0.1;
    
    ci::svg::DocRef         mSvg;
    ci::TriMesh2d           mObstacles;
    
    b2World				*mB2World;
    b2ParticleSystem    *mParticleSystem;

    class ExcNodeNotFound : public cinder::Exception {
    public:
        ExcNodeNotFound(const std::string &name) throw();
        virtual const char* what() const throw() { return mMessage; }
    private:
        char mMessage[2048];
    };

};
