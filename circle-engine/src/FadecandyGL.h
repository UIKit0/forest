// Fadecandy + OpenGL + Cinder adaptor (c) 2015 Micah Elizabeth Scott
// MIT license

#pragma once

#include "OPCClient.h"
#include "cinder/Matrix33.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Vbo.h"
#include "cinder/gl/GlslProg.h"

class FadecandyGL {
public:
    OPCClient opc;

    void setup(ci::app::App &app);
    
    //! One point for each LED, in arbitrary coordinates
    void setModel(const std::vector<ci::Vec2f>& points);

    //! Sample a source texture, updating our framebuffer and transmitting it over OPC
    void update(const ci::gl::Texture& sourceTexture, const ci::Matrix33f& sourceTransform = ci::Matrix33f::identity());

    //! Draw the sampling locations in window coordinates
    void drawModel(const ci::Matrix33f& windowTransform = ci::Matrix33f::identity());
    
    const ci::gl::Texture& getFramebufferTexture() { return mFramebuffer.getTexture(); }

    //! Radius of the sampling circle, in model coordinates
    float samplingRadius;
    
private:
    const unsigned kWidth = 64;
    const unsigned kMinHeight = 64;

    ci::gl::Fbo mFramebuffer;           // LED data, on the GPU
    std::vector<char> mPacketBuffer;    // LED data, in local memory

    ci::gl::Texture mModelTexture;      // Model coordinates, as a texture
    ci::gl::Vbo mModelVbo;              // Model coordinates, as a vertex array
    unsigned mNumPoints;

    ci::gl::GlslProg mProg;             // Sampling shader
};
