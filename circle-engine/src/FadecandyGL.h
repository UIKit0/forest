// Fadecandy + OpenGL + Cinder adaptor (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "OPCClient.h"
#include "cinder/Matrix44.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

class FadecandyGL {
public:
    OPCClient opc;

    void setup();
    void update(const ci::gl::Texture& texture, std::vector<ci::Vec2f> points, const ci::Matrix44f& transform);
    
    const ci::gl::Texture& getFramebufferTexture() { return mFramebuffer.getTexture(); }
    
private:
    static const unsigned kWidth = 64;
    
    ci::gl::Fbo mFramebuffer;
    std::vector<ci::Vec2f> mDrawPoints;
    std::vector<char> mPacketBuffer;
    
    void setupFramebuffer(unsigned numPoints);
};
