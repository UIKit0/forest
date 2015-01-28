// Fadecandy + OpenGL + Cinder adaptor (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "OPCClient.h"
#include "cinder/MatrixAffine2.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/GlslProg.h"

class FadecandyGL {
public:
    OPCClient opc;

    void setup(ci::app::App &app, std::vector<ci::Vec2f>& points, const ci::MatrixAffine2f& transform);
    void update(const ci::gl::Texture& sourceTexture);

    const ci::gl::Texture& getFramebufferTexture() { return mFramebuffer.getTexture(); }
    
private:
    const unsigned kWidth = 64;
    const unsigned kMinHeight = 64;

    ci::gl::Fbo mFramebuffer;
    ci::gl::Texture mModelPoints;
    std::vector<char> mPacketBuffer;
    ci::gl::GlslProg mProg;
    unsigned mNumPoints;
};
